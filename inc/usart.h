#ifndef USART_H
#define USART_H

#include<stdint.h>

void usart_init(uint32_t baud) ; // initialises USART
void usart_write(uint8_t data) ; // writes 8-bit of data to TX buffer
uint8_t usart_read(void) ; // returns 8-bit of data from RX buffer

//debug functions
void uart_print(const char *s);
void uart_println(const char *s);
void uart_print_dec(uint32_t value);
void uart_print_hex8(uint8_t value);
void uart_print_hex16(uint16_t value);
void uart_print_bin8(uint8_t value);

#endif