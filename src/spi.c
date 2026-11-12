#include <avr/io.h>
#include "spi.h"

#define SPI_DDR   DDRB
#define SPI_PORT  PORTB
#define SPI_SCK   PB5
#define SPI_MOSI  PB3
#define SPI_MISO  PB4
#define SPI_CS    PB2

void spi_init(void){
    // CS high BEFORE making it an output, so the chip is never selected by accident 
    SPI_PORT |= (1 << SPI_CS);
    SPI_DDR  |= (1 << SPI_CS) | (1 << SPI_MOSI) | (1 << SPI_SCK);
    SPI_DDR  &= ~(1 << SPI_MISO);
    SPI_PORT |= (1 << SPI_MISO);             // pull-up: floating MISO reads 1s

    // enable SPI, master, mode 0, MSB first, fosc/16 = 1 MHz 
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
    SPSR = 0;                                // SPI2X = 0 
}

uint8_t spi_transfer(uint8_t data){
    SPDR = data;                             // writing SPDR starts the transfer 
    while (!(SPSR & (1 << SPIF)))
        ;                                    // wait for 8 clocks to finish 
    return SPDR;                             // reading SPDR clears SPIF 
}

void spi_cs_low(void) {
    SPI_PORT &= ~(1 << SPI_CS); 
}

void spi_cs_high(void){
    SPI_PORT |=  (1 << SPI_CS); 
}