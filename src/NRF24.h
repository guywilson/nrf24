#ifndef __INCL_NRF24
#define __INCL_NRF24

#define   NRF_TX  0
#define   NRF_RX  1

#define   NRF_ACK_PAYLOAD  -1
#define   NRF_DYNAMIC_PAYLOAD  0
#define   NRF_MIN_PAYLOAD  1
#define   NRF_MAX_PAYLOAD  32

#define   NRF_R_REGISTER           0x00 /* reg in bits 0-4, read 1-5 bytes */
#define   NRF_W_REGISTER           0x20 /* reg in bits 0-4, write 1-5 bytes */
#define   NRF_R_RX_PL_WID          0x60
#define   NRF_R_RX_PAYLOAD         0x61 /* read 1-32 bytes */
#define   NRF_W_TX_PAYLOAD         0xA0 /* write 1-32 bytes */
#define   NRF_W_ACK_PAYLOAD        0xA8 /* pipe in bits 0-2, write 1-32 bytes */
#define   NRF_W_TX_PAYLOAD_NO_ACK  0xB0 /* no ACK, write 1-32 bytes */
#define   NRF_FLUSH_TX             0xE1
#define   NRF_FLUSH_RX             0xE2
#define   NRF_REUSE_TX_PL          0xE3
#define   NRF_NOP                  0xFF /* no operation */

#define   NRF_CONFIG       0x00
#define   NRF_EN_AA        0x01
#define   NRF_EN_RXADDR    0x02
#define   NRF_SETUP_AW     0x03
#define   NRF_SETUP_RETR   0x04
#define   NRF_RF_CH        0x05
#define   NRF_RF_SETUP     0x06
#define   NRF_STATUS       0x07
#define   NRF_OBSERVE_TX   0x08
#define   NRF_RPD          0x09
#define   NRF_RX_ADDR_P0   0x0A
#define   NRF_RX_ADDR_P1   0x0B
#define   NRF_RX_ADDR_P2   0x0C
#define   NRF_RX_ADDR_P3   0x0D
#define   NRF_RX_ADDR_P4   0x0E
#define   NRF_RX_ADDR_P5   0x0F
#define   NRF_TX_ADDR      0x10
#define   NRF_RX_PW_P0     0x11
#define   NRF_RX_PW_P1     0x12
#define   NRF_RX_PW_P2     0x13
#define   NRF_RX_PW_P3     0x14
#define   NRF_RX_PW_P4     0x15
#define   NRF_RX_PW_P5     0x16
#define   NRF_FIFO_STATUS  0x17
#define   NRF_DYNPD        0x1C
#define   NRF_FEATURE      0x1D

/* NRF_CONFIG */
#define   NRF_MASK_RX_DR   1 << 6
#define   NRF_MASK_TX_DS   1 << 5
#define   NRF_MASK_MAX_RT  1 << 4
#define   NRF_EN_CRC       1 << 3 /* default */
#define   NRF_CRCO         1 << 2
#define   NRF_PWR_UP       1 << 1
#define   NRF_PRIM_RX      1 << 0

/* EN_AA */
#define   NRF_ENAA_P5  1 << 5 /* default */
#define   NRF_ENAA_P4  1 << 4 /* default */
#define   NRF_ENAA_P3  1 << 3 /* default */
#define   NRF_ENAA_P2  1 << 2 /* default */
#define   NRF_ENAA_P1  1 << 1 /* default */
#define   NRF_ENAA_P0  1 << 0 /* default */

/* EN_RXADDR */
#define   NRF_ERX_P5  1 << 5
#define   NRF_ERX_P4  1 << 4
#define   NRF_ERX_P3  1 << 3
#define   NRF_ERX_P2  1 << 2
#define   NRF_ERX_P1  1 << 1 /* default */
#define   NRF_ERX_P0  1 << 0 /* default */

/* NRF_SETUP_AW */
#define   NRF_AW_3  1
#define   NRF_AW_4  2
#define   NRF_AW_5  3 /* default */

/* NRF_RF_SETUP */
#define   NRF_CONT_WAVE    1 << 7
#define   NRF_RF_DR_LOW    1 << 5
#define   NRF_PLL_LOCK     1 << 4
#define   NRF_RF_DR_HIGH   1 << 3

/* NRF_STATUS */
#define   NRF_RX_DR        1 << 6
#define   NRF_TX_DS        1 << 5
#define   NRF_MAX_RT       1 << 4

/* RX_P_NO 3-1 */
#define   NRF_TX_FULL      1 << 0

/* NRF_OBSERVE_TX */
/* PLOS_CNT 7-4 */
/* ARC_CNT 3-0 */
/* NRF_RPD */
/* NRF_RPD 1 << 0 */
/* NRF_RX_ADDR_P0 - NRF_RX_ADDR_P5 */

/* NRF_FIFO_STATUS */
#define   NRF_FTX_REUSE  1 << 6
#define   NRF_FTX_FULL   1 << 5
#define   NRF_FTX_EMPTY  1 << 4
#define   NRF_FRX_FULL   1 << 1
#define   NRF_FRX_EMPTY  1 << 0

/* NRF_DYNPD */
#define   NRF_DPL_P5  1 << 5
#define   NRF_DPL_P4  1 << 4
#define   NRF_DPL_P3  1 << 3
#define   NRF_DPL_P2  1 << 2
#define   NRF_DPL_P1  1 << 1
#define   NRF_DPL_P0  1 << 0

/* NRF_FEATURE */
#define   NRF_EN_DPL      1 << 2
#define   NRF_EN_ACK_PAY  1 << 1
#define   NRF_EN_DYN_ACK  1 << 0

typedef struct {
   int sbc;           // sbc connection
   int CE;            // GPIO for chip enable

   int spi_channel;   // SPI channel
   int spi_device;    // SPI device
   int spi_speed;     // SPI bps
   int mode;          // primary mode (RX or TX)
   int channel;       // radio channel
   int payload;       // message size in bytes
   int pad;           // value used to pad short messages
   int address_bytes; // RX/TX address length in bytes
   int crc_bytes;     // number of CRC bytes
   int data_rate;     // Air data rate (1Mbps or 2Mbps)

   const char * local_address;
   const char * remote_address;

   int spih;
   int chip;
   int PTX;
   int CRC;
}
nrf_t;

typedef nrf_t *         nrf_p;

typedef struct {
    float               temperature;
    float               pressure;
    float               humidity;
    float               rainfall;
    float               windspeed;
    uint16_t            windDirection;
}
weather_packet_t;


int         NRF_xfer(nrf_p nrf, char * txBuf, char * rxBuf, int count);
int         NRF_read_register(nrf_p nrf, int reg, char * rxBuf, int count);
int         NRF_write_register(nrf_p nrf, int reg, char * txBuf, int count);
int         NRF_get_status(nrf_p nrf);
void        NRF_set_CE(nrf_p nrf);
void        NRF_unset_CE(nrf_p nrf);
void        NRF_flush_tx(nrf_p nrf);
void        NRF_power_up_tx(nrf_p nrf);
void        NRF_power_up_rx(nrf_p nrf);
void        NRF_configure_payload(nrf_p nrf);
void        NRF_set_channel(nrf_p nrf, int channel);
void        NRF_set_payload(nrf_p nrf, int payload);
void        NRF_set_pad_value(nrf_p nrf, int pad);
void        NRF_set_address_bytes(nrf_p nrf, int address_bytes);
void        NRF_set_CRC_bytes(nrf_p nrf, int crc_bytes);
void        NRF_set_fixed_width(char *data, int *count, int width, int pad);
void        NRF_send(nrf_p nrf, char *data, int count);
void        NRF_ack_payload(nrf_p nrf, char *data, int count);
void        NRF_set_local_address(nrf_p nrf, const char * addr);
void        NRF_set_remote_address(nrf_p nrf, const char * addr);
int         NRF_data_ready(nrf_p nrf);
int         NRF_is_sending(nrf_p nrf);
char *      NRF_get_payload(nrf_p nrf, char *rxBuf);
void        NRF_power_down(nrf_p nrf);
void        NRF_flush_rx(nrf_p nrf);
void        NRF_set_defaults(nrf_p nrf);
void        NRF_init(nrf_p nrf);
void        NRF_term(nrf_p nrf);

nrf_p       getNRFReference();
void        setupNRF24L01();
void *      NRF_listen_thread(void * pParms);

#endif
