#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <strutils.h>

#include "cfgmgr.h"

typedef struct {
    char *          pszKey;
    char *          pszValue;
}
value_map_t;

struct _cfg_handle_t {
    bool            isInstantiated;

    value_map_t *   map;

    int             mapSize;
};

static cfg_handle_t         _cfg;

cfg_handle_t * cfgGetHandle() {
    static cfg_handle_t *       pCfg = NULL;

    if (pCfg == NULL) {
        pCfg = &_cfg;
        pCfg->isInstantiated = false;
    }

    return pCfg;
}

int cfgOpen(const char * pszConfigFileName) {
	FILE *			fptr;
	char *			pszConfigLine;
    char *          pszUntrimmedValue;
	char *			config = NULL;
    char *          config1 = NULL;
    char *          reference = NULL;
	char *			pszCfgItem;
    char *          pszCfgItemFile;
	int				fileLength = 0;
	int				bytesRead = 0;
    int             i;
    int             j;
    int             valueLen = 0;
    int             delimPos = 0;
    int             itemNum = 0;
	const char *	delimiters = "\n\r";

    cfg_handle_t * pCfg = cfgGetHandle();

    if (pCfg->isInstantiated) {
        fprintf(stderr, "Config manager already initialised. You should only call cfgOpen() once\n");
        return 0;
    }

	fptr = fopen(pszConfigFileName, "r");

	if (fptr == NULL) {
		fprintf(stderr, "Failed to open config file '%s': %s\n", pszConfigFileName, strerror(errno));
        return -1;
	}

    fseek(fptr, 0L, SEEK_END);
    fileLength = ftell(fptr);
    rewind(fptr);

	config = (char *)malloc(fileLength + 1);

	if (config == NULL) {
		fprintf(stderr, "Failed to alocate %d bytes for config file '%s'\n", fileLength, pszConfigFileName);
        return -1;
	}

    /*
    ** Retain pointer to original config buffer for 
    ** calling the re-entrant strtok_r() library function...
    */
    reference = &config[0];

	/*
	** Read in the config file...
	*/
	bytesRead = fread(config, 1, fileLength, fptr);

	if (bytesRead != fileLength) {
        fclose(fptr);

		fprintf(
            stderr, 
            "Read %d bytes, but config file '%s' is %d bytes long\n", 
            bytesRead, 
            pszConfigFileName, 
            fileLength);

        return -1;
	}

	fclose(fptr);

    /*
    ** Null terminate the string...
    */
    config[fileLength] = 0;

    config1 = strdup(config);

    pCfg->mapSize = 0;

    /*
    ** Count the number of config items...
    */
	pszConfigLine = strtok_r(config1, delimiters, &reference);

	while (pszConfigLine != NULL) {
        if (pszConfigLine[0] == '#') {
            /*
            ** Ignore line comments...
            */
            pszConfigLine = strtok_r(NULL, delimiters, &reference);
            continue;
        }

        pszConfigLine = strtok_r(NULL, delimiters, &reference);

        pCfg->mapSize++;
    }

    free(config1);

    /*
    ** Retain pointer to original config buffer for 
    ** calling the re-entrant strtok_r() library function...
    */
    reference = &config[0];

    pCfg->map = (value_map_t *)malloc(sizeof(value_map_t) * pCfg->mapSize);

    if (pCfg->map == NULL) {
        fprintf(stderr, "Failed to allocate %ld bytes for config values\n", (sizeof(value_map_t) * pCfg->mapSize));
        free(config);
        return -1;
    }

	pszConfigLine = strtok_r(config, delimiters, &reference);

	while (pszConfigLine != NULL) {
        if (pszConfigLine[0] == '#') {
            /*
            ** Ignore line comments...
            */
            pszConfigLine = strtok_r(NULL, delimiters, &reference);
            continue;
        }

        if (strlen(pszConfigLine) > 0) {
            for (i = 0;i < (int)strlen(pszConfigLine);i++) {
                if (pszConfigLine[i] == '=') {
                    pCfg->map[itemNum].pszKey = strndup(pszConfigLine, i);
                    delimPos = i;
                }
                if (delimPos) {
                    valueLen = strlen(pszConfigLine) - delimPos;

                    for (j = delimPos + 1;j < (int)strlen(pszConfigLine);j++) {
                        if (pszConfigLine[j] == '#') {
                            valueLen = (j - delimPos - 1);
                            break;
                        }
                    }

                    pszUntrimmedValue = strndup(&pszConfigLine[delimPos + 1], valueLen);
                    pCfg->map[itemNum].pszValue = str_trim_trailing(pszUntrimmedValue);

                    /*
                    ** Read the value from the file specified between <>...
                    */
                    if (pCfg->map[itemNum].pszValue[0] == '<' && str_endswith(pCfg->map[itemNum].pszValue, ">")) {
                        pCfg->map[itemNum].pszValue[strlen(pCfg->map[itemNum].pszValue) - 1] = 0;
                        pszCfgItemFile = str_trim(&pCfg->map[itemNum].pszValue[1]);

                        free(pCfg->map[itemNum].pszValue);  

                        FILE * f = fopen(pszCfgItemFile, "rt");

                        if (f == NULL) {
                            free(pszUntrimmedValue);
                            fprintf(
                                stderr, 
                                "Failed to open cfg item file '%s': %s\n", 
                                pszCfgItemFile, 
                                strerror(errno));
                            return -1;
                        }

                        fseek(f, 0L, SEEK_END);
                        long propFileLength = ftell(f);
                        rewind(f);

                        pszCfgItem = (char *)malloc(propFileLength + 1);

                        if (pszCfgItem == NULL) {
                            fprintf(
                                stderr, 
                                "Failed to alocate %ld bytes for config item %s\n", 
                                propFileLength + 1, 
                                pCfg->map[itemNum].pszKey);
                            fclose(f);
                            free(pszUntrimmedValue);

                            return -1;
                        }

                        bytesRead = fread(pszCfgItem, propFileLength, 1, f);

                        if (bytesRead != propFileLength) {
                            fclose(f);
                            free(pszCfgItem);
                            free(pszUntrimmedValue);

                            fprintf(
                                stderr, 
                                "Read %d bytes, but props file '%s' is %ld bytes long\n", 
                                bytesRead, 
                                pszCfgItemFile, 
                                propFileLength);

                            return -1;
                        }

                        pszCfgItem[propFileLength] = 0;
						
						/*
						** Trim the value read from the property file
						*/
						pCfg->map[itemNum].pszValue = str_trim(pszCfgItem);
						
						free(pszCfgItem);

                        fclose(f);
                    }
                    free(pszUntrimmedValue);
                    break;
                }
            }

            delimPos = 0;
            valueLen = 0;

            itemNum++;
        }

        pszConfigLine = strtok_r(NULL, delimiters, &reference);
	}

	free(config);

    pCfg->isInstantiated = true;

    return 0;
}

void cfgClose(cfg_handle_t * hcfg) {
    int         i;

    hcfg->isInstantiated = false;

    for (i = 0;i < hcfg->mapSize;i++) {
        free(hcfg->map[i].pszKey);
        free(hcfg->map[i].pszValue);
    }
    free(hcfg->map);
}

const char * cfgGetValue(cfg_handle_t * hcfg, const char * key) {
    int         i;

    if (hcfg->isInstantiated) {
        for (i = 0;i < hcfg->mapSize;i++) {
            if (strncmp(hcfg->map[i].pszKey, key, strlen(hcfg->map[i].pszKey)) == 0) {
                return strdup(hcfg->map[i].pszValue);
            }
        }
    }

    return "";
}

bool cfgGetValueAsBoolean(cfg_handle_t * hcfg, const char * key) {
    const char *        pszValue;

    pszValue = cfgGetValue(hcfg, key);

    return ((strcmp(pszValue, "yes") == 0 || strcmp(pszValue, "true") == 0 || strcmp(pszValue, "on") == 0) ? true : false);
}

int cfgGetValueAsInteger(cfg_handle_t * hcfg, const char * key) {
    const char *        pszValue;

    pszValue = cfgGetValue(hcfg, key);

    return atoi(pszValue);
}

void cfgDumpConfig(cfg_handle_t * hcfg) {
    int         i;

    if (hcfg->isInstantiated) {
        for (i = 0;i < hcfg->mapSize;i++) {
            printf("'%s' = '%s'\n", hcfg->map[i].pszKey, hcfg->map[i].pszValue);
        }
    }
}
