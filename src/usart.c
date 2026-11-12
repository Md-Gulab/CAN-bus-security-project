#include <avr/io.h>
#include<avr/interrupt.h>
#include "usart.h"

#ifndef F_CPU
#error "F_CPU must be defined (e.g. -DF_CPU=16000000UL)"
#endif

//**************************buffer related stuffs************************************
#define RX_BUF_SIZE 64                 // must be a power of 2, max 256 
#define RX_MASK     (RX_BUF_SIZE - 1)

static volatile uint8_t rx_buf[RX_BUF_SIZE];
static volatile uint8_t rx_head;       // written ONLY by the ISR  
static volatile uint8_t rx_tail;       // written ONLY by main code 
static volatile uint8_t rx_overflow;

ISR(USART_RX_vect){
    uint8_t data = UDR0;               // ALWAYS read it: clears RXC0 
    uint8_t next = (rx_head + 1) & RX_MASK;

    if (next != rx_tail) {             // room available?
        rx_buf[rx_head] = data;        // store first... 
        rx_head = next;                // ...then publish 
    } else {
        rx_overflow++;                 // full: drop the byte 
    }
}

//**************************************************************************************** 

void usart_init(uint32_t baud){
    // Double-speed formula, rounded to nearest 
    uint16_t ubrr = (uint16_t)((F_CPU + (baud * 4UL)) / (baud * 8UL) - 1);

    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr & 0xFF);

    UCSR0A = (1 << U2X0);                                       // double speed 
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);       // enable RX + TX 
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);                     // 8N1 
}

void usart_write(uint8_t data){
    while (!(UCSR0A & (1 << UDRE0)))
        ;                                      // wait: TX buffer empty 
    UDR0 = data;
}

uint8_t uart_available(void){
    return (uint8_t)(rx_head - rx_tail) & RX_MASK;
}

uint8_t uart_read_nb(uint8_t *out){
    if (rx_head == rx_tail)            // empty 
        return 0;

    *out = rx_buf[rx_tail];            // copy out first... 
    rx_tail = (rx_tail + 1) & RX_MASK; // ...then free the slot 
    return 1;
}

uint8_t uart_read(void){
    uint8_t b;
    while (!uart_read_nb(&b))
        ;
    return b;
}

uint8_t uart_overflow_count(void){
    return rx_overflow;
}

void uart_print(const char *s){
    while (*s)
        usart_write((uint8_t)*s++);
}

void uart_println(const char *s){
    uart_print(s);
    uart_print("\r\n");
}

// Unsigned decimal, no leading zeros (uint32_t max = 4294967295, 10 digits)
void uart_print_dec(uint32_t value){
    char buf[10];
    uint8_t i = 0;

    if (value == 0) {
        usart_write('0');
        return;
    }
    while (value) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (i)
        usart_write((uint8_t)buf[--i]);      // digits come out reversed 
}

// Two hex digits, e.g. 0x0F prints "0F" 
void uart_print_hex8(uint8_t value){
    static const char hex[] = "0123456789ABCDEF";
    usart_write((uint8_t)hex[value >> 4]);
    usart_write((uint8_t)hex[value & 0x0F]);
}

void uart_print_hex16(uint16_t value){
    uart_print_hex8((uint8_t)(value >> 8));
    uart_print_hex8((uint8_t)(value & 0xFF));
}

// Eight bits, MSB first. Great for dumping registers
void uart_print_bin8(uint8_t value){
    for (int8_t bit = 7; bit >= 0; bit--)
        usart_write((value & (1 << bit)) ? '1' : '0');
}