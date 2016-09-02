#include <msp430.h>
#include <stdint.h>
#include "uart.h"
#include "spi.h"
#include "enc28j60.h"

// Because MSP430G2553 have only 512 bytes of RAM, we
// unfortunately unable to handle frames larger than ~480 bytes.
#define ETHBUFFER_SIZE 440

uint8_t ethbuffer[ETHBUFFER_SIZE];

void main(void) {
  WDTCTL = WDTPW + WDTHOLD;   // Disable watchdog
  BCSCTL1 = CALBC1_16MHZ;     // Set range
  DCOCTL  = CALDCO_16MHZ;     // Set DCO step + modulation

  uart_init();
  uart_write("\n\n");
  spi_init();

  // First ethernet interface

  uart_write("Initializing eth0: ");

  enc_chipSelect(0);
  enc_init();

  uart_write("done\n");

  uart_write("Link: ");
  if(enc_isLinkUp()) uart_write("up\n"); else uart_write("down\n");

  // Second ethernet interface

  uart_write("Initializing eth1: ");

  enc_chipSelect(1);
  enc_init();

  uart_write("done\n");

  uart_write("Link: ");
  if(enc_isLinkUp()) uart_write("up\n"); else uart_write("down\n");

  // Main retransmission loop

  uint16_t len1 = 0, len2 = 0;
  for(;;) {

    enc_chipSelect(0);
    if((len1 = enc_packetReceive (ETHBUFFER_SIZE, ethbuffer)) > 0) {
      enc_chipSelect(1);
      enc_packetSend(len1, ethbuffer);
    }

    enc_chipSelect(1);
    if((len2 = enc_packetReceive (ETHBUFFER_SIZE, ethbuffer)) > 0) {
      enc_chipSelect(0);
      enc_packetSend(len2, ethbuffer);
    }
  }
}
