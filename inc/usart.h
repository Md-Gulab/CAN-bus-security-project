#ifndef USART_H
#define USART_H

#include<stdint.h>

void usart_init(uint32_t baud) ; // initialises USART
void usart_write(uint8_t data) ; // writes 8-bit of data to TX buffer
uint8_t usart_read(void) ; // returns 8-bit of data from RX buffer

#endif