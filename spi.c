#include <msp430.h>
#include <stdint.h>
#include "spi.h"

volatile uint8_t spi_buf = 0;

#define SCLK    BIT5
#define SDI     BIT6
#define SDO     BIT7
#define CS      BIT4

void spi_init() {
  UCB0CTL1 = UCSWRST;
  UCB0CTL0 = UCSYNC + UCSPB + UCMSB + UCCKPH;//UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master
  UCB0CTL1 |= UCSSEL_2;                        // SMCLK
  UCB0BR0 = 0x01;                             // /2
  UCB0BR1 = 0;

  P1SEL  |= SCLK | SDI | SDO;     // P1.6 is MISO and P1.7 is MOSI
  P1SEL2 |= SCLK | SDI | SDO;

  P1DIR |= SCLK | SDO;
  P1DIR &= ~SDI;
  P2DIR |= CS;    // P2.4 as CS (chip select)

  P2OUT |= CS;

  UCB0CTL1 &= ~UCSWRST;  // Initialize USCI state machine
}

void spi_transfer(uint8_t data) {
  while (!(IFG2 & UCB0TXIFG)); // USCI_A0 TX buffer ready?
  UCB0TXBUF = data;            // Send data over SPI to Slave

  while (!(IFG2 & UCB0RXIFG)); // USCI_A0 RX Received?
  spi_buf = UCB0RXBUF;         // Store received data
}

void spi_chipEnable() {
   P2OUT &= ~CS;
}

void spi_chipDisable() {
   P2OUT |= CS;
}
