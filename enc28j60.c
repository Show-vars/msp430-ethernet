#include "enc28j60.h"
#include "enc28j60registers.h"
#include "spi.h"

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

uint8_t readPhyByte (uint8_t address) {
  enc_writeRegByte(MIREGADR, address);
  enc_writeRegByte(MICMD, MICMD_MIIRD);

  while (enc_readRegByte(MISTAT) & MISTAT_BUSY);

  enc_writeRegByte(MICMD, 0x00);
  return enc_readRegByte(MIRD + 1);
}

int enc_isLinkUp() {
    return (readPhyByte(PHSTAT2) >> 2) & 1;
}

void enc_reset() {
  spi_chipEnable();

  spi_transfer(ENC28J60_SOFT_RESET);

  spi_chipDisable();
}
