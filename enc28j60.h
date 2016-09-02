#include <stdint.h>

#ifndef ENC28J60_H
#define ENC28J60_H

#define MAX_FRAMELEN        1518      // Maximum frame length (MTU + 18)

void enc_chipSelect (uint8_t c);

int enc_isLinkUp ();
uint8_t enc_readRegByte (uint8_t address);
void enc_reset ();
void enc_init ();

void enc_packetSend (uint16_t len, uint8_t* buffer);
uint16_t enc_packetReceive (uint16_t bufferSize, uint8_t* buffer);

#endif
