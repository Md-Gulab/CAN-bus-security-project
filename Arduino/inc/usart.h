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

//uart buffer functions
uint8_t uart_available(void);          // bytes waiting 
uint8_t uart_read_nb(uint8_t *out);    // 1 = got a byte, 0 = empty 
uint8_t uart_read(void);               // blocking, now reads the buffer 
uint8_t uart_overflow_count(void);     // bytes dropped because full 

#endif