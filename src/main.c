#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#include"usart.h"

int main(void) {
    usart_init(9600) ;

    for(const char *s = "USART OK!!\r\n" ; *s ; s++) usart_write((uint8_t)*s) ;
    for(const char *p = "Muhammad you did it!!\r\n" ; *p ; p++) usart_write((uint8_t)*p) ;

    while(1){
        usart_write(usart_read()) ;
    }

}