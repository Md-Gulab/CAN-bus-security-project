#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart.h"
#include "timer.h"

int main(void)
{
    DDRB |= (1 << DDB5);              // PB5 (LED) as output
    usart_init(9600);
    timer0_init();
    sei();                            // enable interrupts globally 

    uint32_t t_led = 0, t_print = 0;

    while (1) {
        uint32_t now = millis();

        if (now - t_led >= 500) {
            t_led = now;
            PINB = (1 << PINB5);      // writing 1 to PINx toggles PORTx
        }
        if (now - t_print >= 1000) {
            t_print = now;
            uart_print("t = ");
            uart_print_dec(now);
            uart_println(" ms");
        }
    }
}