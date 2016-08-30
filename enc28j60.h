#include <stdint.h>

#ifndef ENC28J60_H
#define ENC28J60_H

uint8_t enc_readOp (uint8_t op, uint8_t address);
void enc_writeOp (uint8_t op, uint8_t address, uint8_t data);

void enc_setBank (uint8_t address);

uint8_t enc_readRegByte (uint8_t address);
void enc_writeRegByte (uint8_t address, uint8_t data);

uint8_t readPhyByte (uint8_t address);

int enc_isLinkUp();

void enc_reset();

#endif
