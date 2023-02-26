#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <lgpio.h>

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

int main(void) {
    int                 rtn;
    char                rxBuffer[64];
    nrf_t               nrf;
    weather_packet_t    pkt;

	nrf.CE 				= NRF24L01_CE_PIN;
	nrf.spi_device 		= SPI_DEVICE;
	nrf.spi_channel 	    = SPI_CHANNEL;
	nrf.spi_speed 		= SPI_FREQ;
	nrf.mode 			= NRF_RX;
	nrf.channel 		    = NRF24L01_CHANNEL;
	nrf.payload 		    = NRF_MAX_PAYLOAD;
	nrf.pad 			    = 32;
	nrf.address_bytes 	= 5;
	nrf.crc_bytes 		= 2;
	nrf.PTX 			    = 0;

    NRF_init(&nrf);

    NRF_set_local_address(&nrf, NRF24L01_LOCAL_ADDRESS);
    NRF_set_remote_address(&nrf, NRF24L01_REMOTE_ADDRESS);

	rtn = NRF_read_register(&nrf, NRF24L01_REG_CONFIG, rxBuffer, 1);

    if (rtn < 0) {
        printf(
            "Failed to transfer SPI data: %s\n", 
            lguErrorText(rtn));

        return -1;
    }

    printf("Read back CONFIG reg: 0x%02X\n", (int)rxBuffer[0]);

    if (rxBuffer[0] == 0x00) {
        printf("Config read back as 0x00, device is probably not plugged in?\n\n");
        return -1;
    }

    int i = 0;

    while (i < 60) {
        while (NRF_data_ready(&nrf)) {
            NRF_get_payload(&nrf, rxBuffer);

            hexDump(rxBuffer, NRF_MAX_PAYLOAD);

            // printf("Got temperature: %s\n", rxBuffer);
            memcpy(&pkt, rxBuffer, sizeof(weather_packet_t));

            printf("Got weather data:\n");
            printf("\tTemperature: %.2f\n", pkt.temperature);
            printf("\tPressure:    %.2f\n", pkt.pressure);
            printf("\tHumidity:    %.2f\n\n", pkt.humidity);

            sleep(1);
        }

        i++;

        sleep(2);
    }

	NRF_term(&nrf);

	return 0;
}
