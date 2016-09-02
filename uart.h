#include <stdint.h>

#ifndef UART_H
#define UART_H

void uart_init();
void uart_write(char str[]);
void uart_printHex(uint8_t n);

#endif
