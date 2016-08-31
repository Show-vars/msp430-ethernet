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
  UCA0BR0   =  104;                 // 104 From datasheet table-
  UCA0BR1   =  0;                   // -selects baudrate = 9600, clk = SMCLK
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

uint8_t ethpacket[] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  // Destination MAC address
  0x8e, 0xcc, 0xaa, 0x36, 0x8c, 0x56,  // Source MAC address (individual and localy administrated)
  0x08, 0x00,                          // Type/length (0x0800 Ethernet II standart frame with IPv4 ethertype)

  // Data (64 bytes long)
   0,  1,  2,  3,  4,  5,  6,  7,
   8,  9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23,
  24, 25, 26, 27, 28, 29, 30, 31,
  32, 33, 34, 35, 36, 37, 38, 39,
  40, 41, 42, 43, 44, 45, 46, 47,
  48, 49, 50, 51, 52, 53, 54, 55,
  56, 57, 58, 59, 60, 61, 62, 63
}; // 78 bytes total

void main(void) {
  WDTCTL = WDTPW + WDTHOLD;   // Disable watchdog
  BCSCTL1 = CALBC1_1MHZ;      // Set range
  DCOCTL  = CALDCO_1MHZ;      // Set DCO step + modulation


  uart_init();
  uart_write("INITIALIZING: ");

  spi_init();
  __delay_cycles(1000000);
  enc_init();

  uart_write("DONE\n");

  uint8_t link = enc_isLinkUp();

  uart_write("LINK: ");
  if(link) uart_write("UP\n"); else uart_write("DOWN\n");

  uint16_t ledConf = enc_readPhy(PHLCON);

  uart_write("LEDCONF: ");
  uart_printHex(ledConf >> 8);
  uart_write(" ");
  uart_printHex(ledConf);
  uart_write("\n");

  for(;;) {
    uart_write("SEND PACKET: ");
    enc_packetSend(78, ethpacket);
    uart_write("DONE\n");

    __delay_cycles(1000000);
  }
}
