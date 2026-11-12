#ifndef MCP2515_H
#define MCP2515_H

#define MCP_CNF3          0x28
#define MCP_CNF2          0x29
#define MCP_CNF1          0x2A

#define MCP_MODE_NORMAL   0x00
#define MCP_MODE_SLEEP    0x20
#define MCP_MODE_LOOPBACK 0x40
#define MCP_MODE_LISTEN   0x60
#define MCP_MODE_CONFIG   0x80

#define MCP_CANINTF    0x2C
#define MCP_TXB0CTRL   0x30
#define MCP_TXB0SIDH   0x31
#define MCP_TXB0SIDL   0x32
#define MCP_TXB0DLC    0x35
#define MCP_TXB0D0     0x36
#define MCP_RXB0CTRL   0x60
#define MCP_RXB0SIDH   0x61
#define MCP_RXB0SIDL   0x62
#define MCP_RXB0DLC    0x65
#define MCP_RXB0D0     0x66

uint8_t mcp2515_send_std(uint16_t id, const uint8_t *data, uint8_t len);
uint8_t mcp2515_receive_std(uint16_t *id, uint8_t *data, uint8_t *len);

void    mcp2515_set_bitrate_500k_8mhz(void);   // call in config mode 
uint8_t mcp2515_set_mode(uint8_t mode);        // 1 = success, 0 = timed out 

#include <stdint.h>

#define MCP_CANSTAT  0x0E
#define MCP_CANCTRL  0x0F

void    mcp2515_reset(void);
uint8_t mcp2515_read_reg(uint8_t addr);
void    mcp2515_write_reg(uint8_t addr, uint8_t value);
void    mcp2515_bit_modify(uint8_t addr, uint8_t mask, uint8_t value);

#endif