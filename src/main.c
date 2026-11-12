#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart.h"

int main(void)
{
    usart_init(9600);
    sei();                              // without this, no ISR ever runs 

    uart_println("Type now, 3 s stall...");
    _delay_ms(3000);                    // CPU is busy, not reading 

    uart_print("got ");
    uart_print_dec(uart_available());
    uart_println(" bytes:");

    while (uart_available())
        usart_write(uart_read());
    uart_println("");

    uart_print("overflow = ");
    uart_print_dec(uart_overflow_count());
    uart_println("");

    while (1)
        ;
}