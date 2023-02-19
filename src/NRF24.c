/*
NRF24.c
2020-12-30
Public Domain

http://abyz.me.uk/lg/lgpio.html

gcc -Wall -o NRF24 NRF24.c -llgpio

./NRF24   # rx
./NRF24 x # tx
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#include "dummy_lgpio.h"
#else
#include <lgpio.h>
#endif

#include "nRF24L01.h"
#include "NRF24.h"

/*
   Note that RX and TX addresses must match

   Note that communication channels must match:

   Note that payload size must match:

   The following table describes how to configure the operational
   modes.

   +----------+--------+---------+--------+-----------------------------+
   |Mode      | PWR_UP | PRIM_RX | CE pin | FIFO state                  |
   +----------+--------+---------+--------+-----------------------------+
   |RX mode   |  1     |  1      |  1     | ---                         |
   +----------+--------+---------+--------+-----------------------------+
   |TX mode   |  1     |  0      |  1     | Data in TX FIFOs. Will empty|
   |          |        |         |        | all levels in TX FIFOs      |
   +----------+--------+---------+--------+-----------------------------+
   |TX mode   |  1     |  0      |  >10us | Data in TX FIFOs. Will empty|
   |          |        |         |  pulse | one level in TX FIFOs       |
   +----------+--------+---------+--------+-----------------------------+
   |Standby-II|  1     |  0      |  1     | TX FIFO empty               |
   +----------+--------+---------+--------+-----------------------------+
   |Standby-I |  1     |  ---    |  0     | No ongoing transmission     |
   +----------+--------+---------+--------+-----------------------------+
   |Power Down|  0     |  ---    |  ---   | ---                         |
   +----------+--------+---------+--------+-----------------------------+
*/


int NRF_xfer(nrf_p nrf, char * txBuf, char * rxBuf, int count) {
   return lgSpiXfer(nrf->spih, txBuf, rxBuf, count);
}

int NRF_read_register(nrf_p nrf, int reg, char * rxBuf, int count) {
   int         i;
   char        txBuf[64];
   char        buf[64];

   txBuf[0] = reg;

   for (i = 1;i <= count;i++) {
      txBuf[i] = 0;
   }

   i = NRF_xfer(nrf, txBuf, buf, count + 1);

   if (i >= 0) {
      for (i = 0;i < count;i++) {
         rxBuf[i] = buf[i + 1];
      }

      return count;
   }

   return i;
}

int NRF_write_register(nrf_p nrf, int reg, char * txBuf, int count) {
   int         i;
   char        buf[64];
   char        rxBuf[64];

   buf[0] = NRF_W_REGISTER | reg;

   for (i = 0;i < count;i++) {
      buf[i + 1] = txBuf[i];
   }

   i = NRF_xfer(nrf, buf, rxBuf, count + 1);

   return i;
}

int NRF_get_status(nrf_p nrf) {
   int         res;
   char        txBuf[8];
   char        rxBuf[8];

   txBuf[0] = NRF_NOP;

   res = NRF_xfer(nrf, txBuf, rxBuf, 1);

   if (res >= 0) {
      res = rxBuf[0];
   }

   return res;
}

void NRF_set_CE(nrf_p nrf) {
   lgGpioWrite(nrf->chip, nrf->CE, 1);
}

void NRF_unset_CE(nrf_p nrf) {
   lgGpioWrite(nrf->chip, nrf->CE, 0);
}

void NRF_flush_tx(nrf_p nrf) {
   char        txBuf[64];
   char        rxBuf[64];

   txBuf[0] = NRF_FLUSH_TX;

   NRF_xfer(nrf, txBuf, rxBuf, 1);
}

void NRF_power_up_tx(nrf_p nrf) {
   char        txBuf[128];

   NRF_unset_CE(nrf);

   nrf->PTX = 1;

   txBuf[0] = NRF_EN_CRC | nrf->CRC | NRF_PWR_UP;
   NRF_write_register(nrf, NRF_CONFIG, txBuf, 1);
   NRF_write_register(nrf, NRF_CONFIG, txBuf, 1);

   txBuf[0] = NRF_RX_DR | NRF_TX_DS | NRF_MAX_RT;
   NRF_write_register(nrf, NRF_STATUS, txBuf, 1);

   NRF_set_CE(nrf);
}

void NRF_power_up_rx(nrf_p nrf) {
   char        txBuf[64];

   NRF_unset_CE(nrf);

   nrf->PTX = 0;

   txBuf[0] = NRF_EN_CRC | nrf->CRC | NRF_PWR_UP | NRF_PRIM_RX;

   NRF_write_register(nrf, NRF_CONFIG, txBuf, 1);

   txBuf[0] = NRF_RX_DR | NRF_TX_DS | NRF_MAX_RT;

   NRF_write_register(nrf, NRF_STATUS, txBuf, 1);

   NRF_set_CE(nrf);
}

void NRF_configure_payload(nrf_p nrf) {
   char        txBuf[256];

   if (nrf->payload >= NRF_MIN_PAYLOAD) {
      txBuf[0] = nrf->payload;
      NRF_write_register(nrf, NRF_RX_PW_P0, txBuf, 1);
      NRF_write_register(nrf, NRF_RX_PW_P1, txBuf, 1);

      txBuf[0] = 0;
      NRF_write_register(nrf, NRF_DYNPD, txBuf, 1);
      NRF_write_register(nrf, NRF_FEATURE, txBuf, 1);
   }
   else {
      txBuf[0] = NRF_DPL_P0 | NRF_DPL_P1;
      NRF_write_register(nrf, NRF_DYNPD, txBuf, 1);

      if (nrf->payload  == NRF_ACK_PAYLOAD) {
         txBuf[0]= NRF_EN_DPL | NRF_EN_ACK_PAY;
      }
      else {
         txBuf[0] = NRF_EN_DPL;
      }

      NRF_write_register(nrf, NRF_FEATURE, txBuf, 1);
   }
}

void NRF_set_channel(nrf_p nrf, int channel) {
   char        txBuf[8];

   nrf->channel = channel; // frequency (2400 + channel) MHz

   txBuf[0] = channel;
   NRF_write_register(nrf, NRF_RF_CH, txBuf, 1);
}

void NRF_set_payload(nrf_p nrf, int payload) {
   nrf->payload = payload;  // 0 is dynamic payload
   NRF_configure_payload(nrf);
}

void NRF_set_pad_value(nrf_p nrf, int pad) {
   nrf->pad = pad;
}

void NRF_set_address_bytes(nrf_p nrf, int address_bytes) {
   char        txBuf[8];

   nrf->address_bytes = address_bytes;

   txBuf[0] = nrf->address_bytes - 2;
   NRF_write_register(nrf, NRF_SETUP_AW, txBuf, 1);
}

void NRF_set_CRC_bytes(nrf_p nrf, int crc_bytes) {
   if (crc_bytes == 1) {
      nrf->CRC = 0;
   }
   else {
      nrf->CRC = NRF_CRCO;
   }
}

void NRF_set_fixed_width(char *data, int *count, int width, int pad) {
   int         i;

   for (i = *count;i < width;i++) {
      data[i] = pad;
   }

   *count = width;
}

void NRF_send(nrf_p nrf, char * data, int count) {
   int         status;
   int         i;
   int         n = count;
   char        txBuf[256];

   status = NRF_get_status(nrf);

   if (status & (NRF_TX_FULL | NRF_MAX_RT)) {
      NRF_flush_tx(nrf);
   }

   if (nrf->payload >= NRF_MIN_PAYLOAD) {
      NRF_set_fixed_width(data, &n, nrf->payload, nrf->pad);
   }

   txBuf[0] = NRF_W_TX_PAYLOAD;

   for (i = 0;i < n;i++) {
      txBuf[i+1] = data[i];
   }

   NRF_xfer(nrf, txBuf, NULL, n + 1);

   NRF_power_up_tx(nrf);
}

void NRF_ack_payload(nrf_p nrf, char * data, int count) {
   int         i;
   char        txBuf[256];

   txBuf[0] = NRF_W_ACK_PAYLOAD;

   for (i = 0;i < count;i++) {
      txBuf[i+1] = data[i];
   }

   NRF_xfer(nrf, txBuf, NULL, count+1);
}

void NRF_set_local_address(nrf_p nrf, char * addr) {
   int         i;
   int         n;
   char        txBuf[64];

   n = strlen(addr);

   for (i = 0;i < n;i++) {
      txBuf[i] = addr[i];
   }

   NRF_set_fixed_width(txBuf, &n, nrf->address_bytes, nrf->pad);

   NRF_unset_CE(nrf);

   NRF_write_register(nrf, NRF_RX_ADDR_P1, txBuf, n);

   NRF_set_CE(nrf);
}

void NRF_set_remote_address(nrf_p nrf, char *addr) {
   int         i;
   int         n;
   char        txBuf[64];

   n = strlen(addr);

   for (i = 0;i < n;i++) {
      txBuf[i] = addr[i];
   }

   NRF_set_fixed_width(txBuf, &n, nrf->address_bytes, nrf->pad);

   NRF_unset_CE(nrf);

   NRF_write_register(nrf, NRF_TX_ADDR, txBuf, n);
   NRF_write_register(nrf, NRF_RX_ADDR_P0, txBuf, n); // Needed for auto acks

   NRF_set_CE(nrf);
}

int NRF_data_ready(nrf_p nrf) {
   int         status;
   char        rxBuf[8];

   status = NRF_get_status(nrf);

   if (status & NRF_RX_DR) {
      return 1;
   }

   NRF_read_register(nrf, NRF_FIFO_STATUS, rxBuf, 1);

   status = rxBuf[0];

   return ((status & NRF_FRX_EMPTY) ? 0 : 1);
}

int NRF_is_sending(nrf_p nrf) {
   int         status;

   if (nrf->PTX > 0) {
      status = NRF_get_status(nrf);

      if  (status & (NRF_TX_DS | NRF_MAX_RT)) {
         NRF_power_up_rx(nrf);
         return 0;
      }
      else {
         return 1;
      }
   }

   return 0;
}

char *NRF_get_payload(nrf_p nrf, char * rxBuf) {
   char        txBuf[64];
   int         count;

   txBuf[0] = NRF_R_RX_PL_WID;
   txBuf[1] = 0;

   if (nrf->payload < NRF_MIN_PAYLOAD) {
      NRF_xfer(nrf, txBuf, rxBuf, 2);
      count = rxBuf[1];
   }
   else {
      count = nrf->payload;
   }

   NRF_read_register(nrf, NRF_R_RX_PAYLOAD, rxBuf, count);

   rxBuf[count] = 0;

   NRF_unset_CE(nrf); // added

   txBuf[0] = NRF_RX_DR;
   NRF_write_register(nrf, NRF_STATUS, txBuf, 1);

   NRF_set_CE(nrf); // added

   return rxBuf;
}

void NRF_power_down(nrf_p nrf) {
   char        txBuf[64];

   NRF_unset_CE(nrf);

   txBuf[0] = NRF_EN_CRC | nrf->CRC;

   NRF_write_register(nrf, NRF_CONFIG, txBuf, 1);
}

void NRF_flush_rx(nrf_p nrf) {
   char        txBuf[64];
   char        rxBuf[64];

   txBuf[0] = NRF_FLUSH_RX;
   NRF_xfer(nrf, txBuf, rxBuf, 1);
}

void NRF_set_defaults(nrf_p nrf) {
   nrf->spi_channel=0;   // SPI channel
   nrf->spi_device=0;    // SPI device
   nrf->spi_speed=50e3;  // SPI bps
   nrf->mode=NRF_RX;     // primary mode (RX or TX)
   nrf->channel=1;       // radio channel
   nrf->payload=8;       // message size in bytes
   nrf->pad=32;          // value used to pad short messages
   nrf->address_bytes=5; // RX/TX address length in bytes
   nrf->crc_bytes=1;     // number of CRC bytes

   nrf->sbc=-1;          // sbc connection
   nrf->CE=-1;           // GPIO for chip enable
   nrf->spih=-1;         // SPI handle
   nrf->chip=-1;         // gpiochip handle
   nrf->PTX=-1;          // RX or TX
   nrf->CRC=-1;          // CRC bytes
}

void NRF_init(nrf_p nrf) {
   char txBuf[256];

   nrf->chip = lgGpiochipOpen(0);

   lgGpioClaimOutput(nrf->chip, 0, nrf->CE, 0);

   NRF_unset_CE(nrf);

   nrf->spih = lgSpiOpen(
                     nrf->spi_channel, 
                     nrf->spi_device, 
                     nrf->spi_speed, 
                     0);

   NRF_set_channel(nrf, nrf->channel);

   NRF_set_payload(nrf, nrf->payload);

   NRF_set_pad_value(nrf, nrf->pad);

   NRF_set_address_bytes(nrf, nrf->address_bytes);

   NRF_set_CRC_bytes(nrf, nrf->crc_bytes);

   nrf->PTX = 0;

   NRF_power_down(nrf);

   txBuf[0] = 0x1F;

   NRF_write_register(nrf, NRF_SETUP_RETR, txBuf, 1);

   txBuf[0] = 
         NRF24L01_RF_SETUP_RF_POWER_MEDIUM | 
         NRF24L01_RF_SETUP_RF_LNA_GAIN_ON | 
         NRF24L01_RF_SETUP_DATA_RATE_1MBPS;

   NRF_write_register(nrf, NRF24L01_REG_RF_SETUP, txBuf, 1);

   txBuf[0] = 0x00;

   NRF_write_register(nrf, NRF_EN_AA, txBuf, 1);
   
   NRF_flush_rx(nrf);
   NRF_flush_tx(nrf);

   NRF_power_up_rx(nrf);
}

void NRF_term(nrf_p nrf) {
   NRF_power_down(nrf);

   lgSpiClose(nrf->spih);

   lgGpioFree(nrf->chip, nrf->CE);

   lgGpiochipClose(nrf->chip);
}

// int main(int argc, char *argv[])
// {
//    int sending;
//    int count;
//    double end_time;
//    char *ver = "l";
//    char s[256];
//    nrf_t nrf;

//    if (argc > 1) sending = 1;
//    else sending = 0;

//    NRF_set_defaults(&nrf);

//    nrf.CE = 27;

//    nrf.payload = NRF_ACK_PAYLOAD;
//    nrf.pad = '*';
//    nrf.address_bytes=3;
//    nrf.crc_bytes = 2;

//    NRF_init(&nrf);

//    NRF_show_registers(&nrf);

//    end_time = lguTime() + 3600;

//    if (sending)
//    {
//       count = 0;

//       NRF_set_local_address(&nrf, "h1");
//       NRF_set_remote_address(&nrf, "h2");

//       while (lguTime() < end_time)
//       {
//          //printf("%s\n", NRF_format_FIFO_STATUS(&nrf, s));
//          //printf("%s\n", NRF_format_OBSERVE_TX(&nrf, s));

//          if (!NRF_is_sending(&nrf))
//          {
//             printf("%s> %s\n", ver, test_words[count]);
//             NRF_send(&nrf, test_words[count], strlen(test_words[count]));
//             count += 1;
//             if (count >= number_of_test_words) count = 0;
//          }

//          lguSleep(0.5);

//       }
//    }
//    else
//    {
//       NRF_set_local_address(&nrf, "h2");
//       NRF_set_remote_address(&nrf,"h1");

//       while (lguTime() < end_time)
//       {
//          //printf("%s\n", NRF_format_FIFO_STATUS(&nrf, s));
//          //printf("%s\n", NRF_format_OBSERVE_TX(&nrf, s));

//          while (NRF_data_ready(&nrf))
//          {
//             printf("%s< %s\n", ver, NRF_get_payload(&nrf, s));
//          }
//          lguSleep(0.5);
//       }
//    }

//    NRF_term(&nrf);

//    return 0;
// }

