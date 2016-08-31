#include "enc28j60.h"
#include "enc28j60registers.h"
#include "spi.h"

#define RXSTART_INIT        0x0000  // start of RX buffer, (must be zero, Rev. B4 Errata point 5)
#define RXSTOP_INIT         0x0BFF  // end of RX buffer, room for 2 packets

#define TXSTART_INIT        0x0C00  // start of TX buffer, room for 1 packet
#define TXSTOP_INIT         0x11FF  // end of TX buffer

#define MAX_FRAMELEN        1518      // Maximum frame length

uint8_t enc_bank = 0;

uint8_t enc_readOp (uint8_t op, uint8_t address) {
  spi_chipEnable();

  spi_transfer(op | (address & ADDR_MASK));
  spi_transfer(0x00);

  if (address & 0x80) spi_transfer(0x00);

  uint8_t result = spi_buf;

  spi_chipDisable();

  return result;
}

void enc_writeOp (uint8_t op, uint8_t address, uint8_t data) {
  spi_chipEnable();

  spi_transfer(op | (address & ADDR_MASK));
  spi_transfer(data);

  spi_chipDisable();
}

void enc_setBank (uint8_t address) {
  if ((address & BANK_MASK) != enc_bank) {
    enc_writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_BSEL1|ECON1_BSEL0);
    enc_bank = address & BANK_MASK;
    enc_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, enc_bank >> 5);
  }
}

uint8_t enc_readRegByte (uint8_t address) {
  enc_setBank(address);
  return enc_readOp(ENC28J60_READ_CTRL_REG, address);
}

void enc_writeRegByte (uint8_t address, uint8_t data) {
  enc_setBank(address);
  enc_writeOp(ENC28J60_WRITE_CTRL_REG, address, data);
}

void enc_writeReg(uint8_t address, uint16_t data) {
    enc_writeRegByte(address, data);
    enc_writeRegByte(address + 1, data >> 8);
}

uint8_t enc_readPhyByte (uint8_t address) {
  enc_writeRegByte(MIREGADR, address);
  enc_writeRegByte(MICMD, MICMD_MIIRD);

  while (enc_readRegByte(MISTAT) & MISTAT_BUSY);

  enc_writeRegByte(MICMD, 0x00);
  return enc_readRegByte(MIRD + 1);
}

void enc_writePhy (uint8_t address, uint16_t data) {
    enc_writeRegByte(MIREGADR, address);
    enc_writeReg(MIWR, data);
    while (enc_readRegByte(MISTAT) & MISTAT_BUSY);
}

int enc_isLinkUp() {
    return (enc_readPhyByte(PHSTAT2) >> 2) & 1;
}

void enc_reset() {
  spi_chipEnable();
  spi_transfer(ENC28J60_SOFT_RESET);
  spi_chipDisable();

  while (!enc_readOp(ENC28J60_READ_CTRL_REG, ESTAT) & ESTAT_CLKRDY);
}

void enc_init() {
  enc_reset();

  // Receive buffer bounds
  enc_writeReg(ERXST, RXSTART_INIT);
  enc_writeReg(ERXRDPT, RXSTART_INIT);
  enc_writeReg(ERXND, RXSTOP_INIT);

  // Transmit buffer bounds
  enc_writeReg(ETXST, TXSTART_INIT);
  enc_writeReg(ETXND, TXSTOP_INIT);

  // Receive filters (enable CRC check only)
  enc_writeRegByte(ERXFCON, ERXFCON_CRCEN);

  // PHY configuring (enable Full-Duplex)
  enc_writePhy(PHCON1, PHCON1_PDPXMD);

  // Mac configuring (Enable packet receiving, enable receive and transmit Pause Control Frame)
  enc_writeRegByte(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
  // writeRegByte(MACON2, 0x00); Register is just reserved
  enc_writeRegByte(MACON3, MACON3_FULDPX);

  // Back-to-Back Inter-Packet Gap. We use Full-Duplex, so set 15h as sed datasheet
  enc_writeRegByte(MABBIPG, 0x15);

  // Max ethernet frame length
  enc_writeReg(MAMXFL, MAX_FRAMELEN);

  // Setting global interrupts (Enable Global INT Interrupt and Receive Packet Pending Interrupt)
  enc_writeRegByte(EIE, EIE_INTIE | EIE_PKTIE);

  // Enable Receive
  enc_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}
