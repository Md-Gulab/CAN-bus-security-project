#include <avr/io.h>
#include "usart.h"

#ifndef F_CPU
#error "F_CPU must be defined (e.g. -DF_CPU=16000000UL)"
#endif

void usart_init(uint32_t baud){
    // Double-speed formula, rounded to nearest 
    uint16_t ubrr = (uint16_t)((F_CPU + (baud * 4UL)) / (baud * 8UL) - 1);

    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr & 0xFF);

    UCSR0A = (1 << U2X0);                      // double speed 
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);      // enable RX + TX 
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);    // 8N1 
}

void usart_write(uint8_t data){
    while (!(UCSR0A & (1 << UDRE0)))
        ;                                      // wait: TX buffer empty 
    UDR0 = data;
}

uint8_t usart_read(void){
    while (!(UCSR0A & (1 << RXC0)))
        ;                                      // wait: byte received 
    return UDR0;
}