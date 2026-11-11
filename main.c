#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    DDRB |= (1 << DDB5);          // PB5 = Uno pin 13 (onboard LED) as output
    while (1) {
        PORTB ^= (1 << PORTB5);   // toggle
        _delay_ms(300);
    }
}