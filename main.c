#include <msp430.h>
#include <stdint.h>
#include "spi.h"
#include "enc28j60.h"
#include "enc28j60registers.h"

#define LED_0 BIT0
#define LED_1 BIT6
#define LED_OUT P1OUT
#define LED_DIR P1DIR

void uart_init() {
  P1SEL  |=  BIT1 + BIT2;  // P1.1 UCA0RXD input
  P1SEL2 |=  BIT1 + BIT2;  // P1.2 UCA0TXD output

  UCA0CTL1 |=  UCSSEL_2 + UCSWRST;  // USCI Clock = SMCLK,USCI_A0 disabled
  UCA0BR0   =  0x82;                 // 104 From datasheet table-
  UCA0BR1   =  0x06;                   // -selects baudrate = 9600, clk = SMCLK
  UCA0MCTL  =  UCBRS_1;             // Modulation value = 1 from datasheet
  UCA0STAT |=  UCLISTEN;            // loop back mode enabled
  UCA0CTL1 &= ~UCSWRST;             // Clear UCSWRST to enable USCI_A0
}

void uart_write(char str[]) {
  for(int i = 0; str[i] != '\0'; i++) {
    while (!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = str[i];
  }
}

void uart_printHex(uint8_t n) {
  char buf[2 + 1];
  char *str = &buf[3 - 1];

  *str = '\0';

  uint8_t base = 16;

  do {
    uint8_t m = n;
    n /= base;
    char c = m - base * n;
    *--str = c < 10 ? c + '0' : c + 'A' - 10;
  } while(n);

  uart_write(str);
}

#define ETHBUFFER_SIZE 440

uint8_t ethbuffer[ETHBUFFER_SIZE];

void main(void) {
  WDTCTL = WDTPW + WDTHOLD;   // Disable watchdog
  BCSCTL1 = CALBC1_16MHZ;      // Set range
  DCOCTL  = CALDCO_16MHZ;      // Set DCO step + modulation


  uart_init();
  spi_init();

  __delay_cycles(1000000);

  uart_write("INITIALIZING 1: ");


  enc_chipSelect(0); // First interface
  enc_init();

  uart_write("DONE\n");

  uint8_t link1;

  uart_write("LINK 1: ");
  link1 = enc_isLinkUp();
  if(link1) uart_write("UP\n"); else uart_write("DOWN\n");

  __delay_cycles(500000);

  uart_write("INITIALIZING 2: ");

  enc_chipSelect(1); // Second interface
  enc_init();

  uart_write("DONE\n");

  uint8_t link2;

  uart_write("LINK 2: ");
  link2 = enc_isLinkUp();
  if(link2) uart_write("UP\n"); else uart_write("DOWN\n");

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

    /*
    uart_write("SEND PACKET 1: ");
    enc_chipSelect(0);
    enc_packetSend(78, ethpacket1);
    uart_write("DONE\n");

    __delay_cycles(500000);

    uart_write("SEND PACKET 2: ");
    enc_chipSelect(1);
    enc_packetSend(78, ethpacket2);
    uart_write("DONE\n");

    __delay_cycles(500000);
    */
  }
}
