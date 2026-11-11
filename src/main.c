#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#include"usart.h"

int main(void) {
    usart_init(9600) ;
    uart_println("Boot OK");

    uart_print("UBRR0 = ");  uart_print_dec(((uint16_t)UBRR0H << 8) | UBRR0L);
    uart_println("");
    uart_print("UCSR0B = 0b"); uart_print_bin8(UCSR0B);
    uart_println("");
    uart_print("UCSR0C = 0x");  uart_print_hex8(UCSR0C);
    uart_println("");

    uint32_t count = 0;
    while(1){
        uart_print("count = ");
        uart_print_dec(count++);
        uart_println("");
        _delay_ms(500);
    }

}