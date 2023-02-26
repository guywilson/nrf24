#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <lgpio.h>

#include "cfgmgr.h"
#include "logger.h"
#include "posixthread.h"
#include "timeutils.h"
#include "nRF24L01.h"
#include "NRF24.h"

typedef struct {
    float               temperature;
    float               pressure;
    float               humidity;
    float               rainfall;
    float               windspeed;
    uint16_t            windDirection;
}
weather_packet_t;

#define NRF24L01_CE_PIN                     25
#define NRF24L01_CHANNEL                    40
#define SPI_DEVICE                           0
#define SPI_CHANNEL                          0
#define SPI_FREQ                       4000000

#define NRF24L01_LOCAL_ADDRESS          "AZ438"
#define NRF24L01_REMOTE_ADDRESS         "AZ437"

void hexDump(void * buffer, uint32_t bufferLen)
{
    int         i;
    int         j = 0;
    uint8_t *   buf;
    static char szASCIIBuf[17];

    buf = (uint8_t *)buffer;

    for (i = 0;i < bufferLen;i++) {
        if ((i % 16) == 0) {
            if (i != 0) {
                szASCIIBuf[j] = 0;
                j = 0;

                printf("  |%s|", szASCIIBuf);
            }
                
            printf("\n%08X\t", i);
        }

        if ((i % 2) == 0 && (i % 16) > 0) {
            printf(" ");
        }

        printf("%02X", buf[i]);
        szASCIIBuf[j++] = isprint(buf[i]) ? buf[i] : '.';
    }

    /*
    ** Print final ASCII block...
    */
    szASCIIBuf[j] = 0;
    printf("  |%s|\n", szASCIIBuf);
}

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

int main(int argc, char ** argv) {
	char *			    pszLogFileName = NULL;
	char *			    pszConfigFileName = NULL;
	int				    i;
	bool			    isDaemonised = false;
	bool			    isDumpConfig = false;
	const char *	    defaultLoggingLevel = "LOG_LEVEL_INFO | LOG_LEVEL_ERROR | LOG_LEVEL_FATAL";
    int                 rtn;
    char                rxBuffer[64];
    nrf_t               nrf;
    weather_packet_t    pkt;

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
//					printf("%s Version: [wctl], Build date: [%s]\n\n", getVersion(), getBuildDate());
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
//		daemonise();
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

	nrf.CE 				= NRF24L01_CE_PIN;
	nrf.spi_device 		= SPI_DEVICE;
	nrf.spi_channel 	= SPI_CHANNEL;
	nrf.spi_speed 		= SPI_FREQ;
	nrf.mode 			= NRF_RX;
	nrf.channel 		= NRF24L01_CHANNEL;
	nrf.payload 		= NRF_MAX_PAYLOAD;
	nrf.pad 			= 32;
	nrf.address_bytes 	= 5;
	nrf.crc_bytes 		= 2;
	nrf.PTX 			= 0;

    NRF_init(&nrf);

    NRF_set_local_address(&nrf, NRF24L01_LOCAL_ADDRESS);
    NRF_set_remote_address(&nrf, NRF24L01_REMOTE_ADDRESS);

	rtn = NRF_read_register(&nrf, NRF24L01_REG_CONFIG, rxBuffer, 1);

    if (rtn < 0) {
        lgLogError(lgGetHandle(), "Failed to transfer SPI data: %s\n", lguErrorText(rtn));

        return -1;
    }

    printf("Read back CONFIG reg: 0x%02X\n", (int)rxBuffer[0]);

    if (rxBuffer[0] == 0x00) {
        lgLogError(lgGetHandle(), "Config read back as 0x00, device is probably not plugged in?\n\n");
        return -1;
    }

    i = 0;

    while (i < 60) {
        while (NRF_data_ready(&nrf)) {
            NRF_get_payload(&nrf, rxBuffer);

            hexDump(rxBuffer, NRF_MAX_PAYLOAD);

            memcpy(&pkt, rxBuffer, sizeof(weather_packet_t));

            lgLogDebug(lgGetHandle(), "Got weather data:\n");
            lgLogDebug(lgGetHandle(), "\tTemperature: %.2f\n", pkt.temperature);
            lgLogDebug(lgGetHandle(), "\tPressure:    %.2f\n", pkt.pressure);
            lgLogDebug(lgGetHandle(), "\tHumidity:    %.2f\n\n", pkt.humidity);

            sleep(1);
        }

        i++;

        sleep(2);
    }

	NRF_term(&nrf);
    lgClose(lgGetHandle());
    cfgClose(cfgGetHandle());

	return 0;
}
