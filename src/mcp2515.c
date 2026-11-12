#include <util/delay.h>
#include "mcp2515.h"
#include "spi.h"
#include "timer.h"

#define TXREQ  0x08
#define RX0IF  0x01

void mcp2515_reset(void){
    spi_cs_low();
    spi_transfer(0xC0);
    spi_cs_high();
    _delay_ms(10);                 // oscillator start-up + reset settling 
}

uint8_t mcp2515_read_reg(uint8_t addr){
    spi_cs_low();
    spi_transfer(0x03);
    spi_transfer(addr);
    uint8_t v = spi_transfer(0x00);    // dummy byte clocks the value out 
    spi_cs_high();
    return v;
}

void mcp2515_write_reg(uint8_t addr, uint8_t value){
    spi_cs_low();
    spi_transfer(0x02);
    spi_transfer(addr);
    spi_transfer(value);
    spi_cs_high();
}

void mcp2515_bit_modify(uint8_t addr, uint8_t mask, uint8_t value){
    spi_cs_low();
    spi_transfer(0x05);
    spi_transfer(addr);
    spi_transfer(mask);
    spi_transfer(value);
    spi_cs_high();
}

void mcp2515_set_bitrate_500k_8mhz(void){
    mcp2515_write_reg(MCP_CNF1, 0x00);
    mcp2515_write_reg(MCP_CNF2, 0x90);
    mcp2515_write_reg(MCP_CNF3, 0x02);
}

uint8_t mcp2515_set_mode(uint8_t mode){
    mcp2515_bit_modify(MCP_CANCTRL, 0xE0, mode);

    uint32_t start = millis();
    while ((uint32_t)(millis() - start) < 10) {
        if ((mcp2515_read_reg(MCP_CANSTAT) & 0xE0) == mode)
            return 1;
    }
    return 0;                       // mode change did not happen 
}

// Returns 1 if the frame was sent, 0 on error or timeout 
uint8_t mcp2515_send_std(uint16_t id, const uint8_t *data, uint8_t len){
    if (len > 8)
        return 0;
    if (mcp2515_read_reg(MCP_TXB0CTRL) & TXREQ)
        return 0;                                   // buffer still busy 

    mcp2515_write_reg(MCP_TXB0SIDH, (uint8_t)(id >> 3));
    mcp2515_write_reg(MCP_TXB0SIDL, (uint8_t)((id & 0x07) << 5));
    mcp2515_write_reg(MCP_TXB0DLC,  len);           // RTR bit = 0: data frame 
    for (uint8_t i = 0; i < len; i++)
        mcp2515_write_reg(MCP_TXB0D0 + i, data[i]);

    mcp2515_bit_modify(MCP_TXB0CTRL, TXREQ, TXREQ); // request to send 

    uint32_t start = millis();
    while (mcp2515_read_reg(MCP_TXB0CTRL) & TXREQ) {
        if ((uint32_t)(millis() - start) >= 20)
            return 0;                               // never completed 
    }
    return 1;
}

// Returns 1 if a frame was read, 0 if nothing was waiting 
uint8_t mcp2515_receive_std(uint16_t *id, uint8_t *data, uint8_t *len){
    if (!(mcp2515_read_reg(MCP_CANINTF) & RX0IF))
        return 0;

    uint8_t sidh = mcp2515_read_reg(MCP_RXB0SIDH);
    uint8_t sidl = mcp2515_read_reg(MCP_RXB0SIDL);
    uint8_t dlc  = mcp2515_read_reg(MCP_RXB0DLC) & 0x0F;
    if (dlc > 8)
        dlc = 8;

    *id  = ((uint16_t)sidh << 3) | (sidl >> 5);     // standard frames only 
    *len = dlc;
    for (uint8_t i = 0; i < dlc; i++)
        data[i] = mcp2515_read_reg(MCP_RXB0D0 + i);

    mcp2515_bit_modify(MCP_CANINTF, RX0IF, 0x00);   // release the buffer 
    return 1;
}