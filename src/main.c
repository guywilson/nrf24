#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <lgpio.h>

#include "version.h"
#include "cfgmgr.h"
#include "logger.h"
#include "posixthread.h"
#include "timeutils.h"
#include "nRF24L01.h"
#include "NRF24.h"
#include "utils.h"

static nrf_t               nrf;

void printUsage() {
	printf("\n Usage: wctl [OPTIONS]\n\n");
	printf("  Options:\n");
	printf("   -h/?             Print this help\n");
	printf("   -version         Print the program version\n");
	printf("   -port device     Serial port device\n");
	printf("   -baud baudrate   Serial port baud rate\n");
	printf("   -cfg configfile  Specify the cfg file, default is ./webconfig.cfg\n");
	printf("   -d               Daemonise this application\n");
	printf("   -log  filename   Write logs to the file\n");
	printf("\n");
}

void handleSignal(int sigNum) {
	switch (sigNum) {
		case SIGINT:
			lgLogStatus(lgGetHandle(), "Detected SIGINT, cleaning up...");
			break;

		case SIGTERM:
			lgLogStatus(lgGetHandle(), "Detected SIGTERM, cleaning up...");
			break;

		case SIGUSR1:
			/*
			** We're interpreting this as a request to turn on/off debug logging...
			*/
			lgLogStatus(lgGetHandle(), "Detected SIGUSR1...");

			if (lgCheckLogLevel(lgGetHandle(), LOG_LEVEL_INFO)) {
				int level = lgGetLogLevel(lgGetHandle());
				level &= ~LOG_LEVEL_INFO;
				lgSetLogLevel(lgGetHandle(), level);
			}
			else {
				int level = lgGetLogLevel(lgGetHandle());
				level |= LOG_LEVEL_INFO;
				lgSetLogLevel(lgGetHandle(), level);
			}

			if (lgCheckLogLevel(lgGetHandle(), LOG_LEVEL_DEBUG)) {
				int level = lgGetLogLevel(lgGetHandle());
				level &= ~LOG_LEVEL_DEBUG;
				lgSetLogLevel(lgGetHandle(), level);
			}
			else {
				int level = lgGetLogLevel(lgGetHandle());
				level |= LOG_LEVEL_DEBUG;
				lgSetLogLevel(lgGetHandle(), level);
			}
			return;
	}

    puts("\n");
    
    NRF_term(&nrf);
    lgClose(lgGetHandle());
    cfgClose(cfgGetHandle());

    exit(0);
}

int main(int argc, char ** argv) {
	char *			    pszLogFileName = NULL;
	char *			    pszConfigFileName = NULL;
	int				    i;
    int                 rtn;
	bool			    isDaemonised = false;
	bool			    isDumpConfig = false;
	const char *	    defaultLoggingLevel = "LOG_LEVEL_INFO | LOG_LEVEL_ERROR | LOG_LEVEL_FATAL";
    pxt_handle_t        nrfListenThread;

    tmInitialiseUptimeClock();
	
	if (argc > 1) {
		for (i = 1;i < argc;i++) {
			if (argv[i][0] == '-') {
				if (argv[i][1] == 'd') {
					isDaemonised = true;
				}
				else if (strcmp(&argv[i][1], "log") == 0) {
					pszLogFileName = strdup(&argv[++i][0]);
				}
				else if (strcmp(&argv[i][1], "cfg") == 0) {
					pszConfigFileName = strdup(&argv[++i][0]);
				}
				else if (strcmp(&argv[i][1], "-dump-config") == 0) {
					isDumpConfig = true;
				}
				else if (argv[i][1] == 'h' || argv[i][1] == '?') {
					printUsage();
					return 0;
				}
				else if (strcmp(&argv[i][1], "version") == 0) {
					printf("%s Version: [wctl], Build date: [%s]\n\n", getVersion(), getBuildDate());
					return 0;
				}
				else {
					printf("Unknown argument '%s'", &argv[i][0]);
					printUsage();
					return 0;
				}
			}
		}
	}
	else {
		printUsage();
		return -1;
	}

	if (isDaemonised) {
		daemonise();
	}

    rtn = cfgOpen(pszConfigFileName);

    if (rtn) {
		fprintf(stderr, "Could not read config file: '%s'\n", pszConfigFileName);
		fprintf(stderr, "Aborting!\n\n");
		fflush(stderr);
		exit(-1);
    }
	
	if (pszConfigFileName != NULL) {
		free(pszConfigFileName);
	}

	if (isDumpConfig) {
        cfgDumpConfig(cfgGetHandle());
        cfgClose(cfgGetHandle());
        return 0;
	}

	if (pszLogFileName != NULL) {
        lgOpen(pszLogFileName, defaultLoggingLevel);
		free(pszLogFileName);
	}
	else {
		const char * filename = cfgGetValue(cfgGetHandle(), "log.filename");
		const char * level = cfgGetValue(cfgGetHandle(), "log.level");

		if (strlen(filename) == 0 && strlen(level) == 0) {
			lgOpenStdout(defaultLoggingLevel);
		}
		else if (strlen(level) == 0) {
            lgOpen(filename, defaultLoggingLevel);
		}
		else {
            lgOpen(filename, level);
		}
	}

	/*
	 * Register signal handler for cleanup...
	 */
	if (signal(SIGINT, &handleSignal) == SIG_ERR) {
		lgLogFatal(lgGetHandle(), "Failed to register signal handler for SIGINT");
		return -1;
	}

	if (signal(SIGTERM, &handleSignal) == SIG_ERR) {
		lgLogFatal(lgGetHandle(), "Failed to register signal handler for SIGTERM");
		return -1;
	}

	if (signal(SIGUSR1, &handleSignal) == SIG_ERR) {
		lgLogFatal(lgGetHandle(), "Failed to register signal handler for SIGUSR1");
		return -1;
	}

	if (signal(SIGUSR2, &handleSignal) == SIG_ERR) {
		lgLogFatal(lgGetHandle(), "Failed to register signal handler for SIGUSR2");
		return -1;
	}

    setupNRF24L01();

    pxtCreate(&nrfListenThread, &NRF_listen_thread, false);
    pxtStart(&nrfListenThread, getNRFReference());

    while (1) {
        pxtSleep(seconds, 5);
    }

	return 0;
}
