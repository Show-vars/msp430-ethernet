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

void uart_write(char str[], int length) {
  for(int i = 0; i < length; i++) {
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

  uart_write(str, 2);
}

void main(void) {
  WDTCTL = WDTPW + WDTHOLD;   // Disable watchdog
  BCSCTL1 = CALBC1_1MHZ;      // Set range
  DCOCTL  = CALDCO_1MHZ;      // Set DCO step + modulation


  uart_init();
  uart_write("READY\n", 6);

  spi_init();

  enc_reset();

  __delay_cycles(1000000);
  while (!enc_readOp(ENC28J60_READ_CTRL_REG, ESTAT) & ESTAT_CLKRDY);

  uint8_t link = enc_readRegByte (EREVID);
  //uint8_t link = enc_readOp(ENC28J60_READ_CTRL_REG, ECON2);

  uart_write("LINK:", 5);
  uart_printHex(link);
  if(link == 0x80) uart_write(" UP\n", 4); else uart_write(" DOWN\n", 6);

  for(;;) {}
}
