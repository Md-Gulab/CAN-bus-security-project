#include<avr/io.h>          //defines the register names, bit names
#include<avr/interrupt.h>   //provides the macros for ISR
#include<util/atomic.h>     //provides ATOMIC_BLOCK used in millis()
#include"timer.h"           // header file

static volatile uint32_t g_ms = 0 ;

ISR(TIMER0_COMPA_vect){
    g_ms++ ;    
}

void timer0_init(void){
    TCCR0A = (1 << WGM01) ; //says that clears the timer value once COMPA(comparison) is true
    OCR0A = 249 ; // gives prescaler 64, resulting a 1 millisecond @ 16Mhz
    TCNT0 = 0 ;   // the timer register itself , clears to 0 , even though after reset it is made 0

    /*TIMSK0 is the Timer Interrupt Mask Register, and OCIE0A means Output Compare Match A Interrupt Enable.
        Without this bit the flag OCF0A still gets set, but no ISR runs. 
        Two gates must be open for the ISR to fire: this local enable, 
        and the global one (sei() in main). That’s why timer0_init() doesn’t call sei() itself. 
        The caller decides when interrupts go live. */
    TIMSK0 = (1 << OCIE0A);    

    TCCR0B = (1 << CS01) | (1 << CS00);
    /*The clock-select bits CS02:0 live here. 
    The value 011 selects clk/64. The reset value 000 means the timer is stopped, 
    so this line is what actually starts it. 
    It goes last on purpose: everything else is configured while the timer is stopped, 
    so no compare match can fire half-configured.*/
}

uint32_t millis(void){
    uint32_t ms;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        ms = g_ms;
    }
    return ms;
}
