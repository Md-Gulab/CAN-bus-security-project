#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void    spi_init(void);
uint8_t spi_transfer(uint8_t data);   // send one byte, return the byte received 
void    spi_cs_low(void);             // select the chip   
void    spi_cs_high(void);            // deselect the chip 

#endif