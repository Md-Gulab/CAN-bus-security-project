#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart.h"
#include "spi.h"
#include "mcp2515.h"
#include "timer.h"

int main(void){

    usart_init(9600);
    spi_init();
    timer0_init();
    sei();

    mcp2515_reset();
    mcp2515_set_bitrate_500k_8mhz();
    mcp2515_write_reg(MCP_RXB0CTRL, 0x60);              // accept all frames 
    mcp2515_set_mode(MCP_MODE_LOOPBACK);

    uint8_t tx[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
    uint8_t ok = mcp2515_send_std(0x123, tx, 4);
    uart_print("send ok="); uart_print_dec(ok); uart_println("");

    uint16_t id; uint8_t rx[8]; uint8_t len;
    if (mcp2515_receive_std(&id, rx, &len)) {
        uart_print("rx id=0x"); uart_print_hex16(id);
        uart_print(" len=");    uart_print_dec(len);
        uart_print(" data:");
        for (uint8_t i = 0; i < len; i++) {
            usart_write(' ');
            uart_print_hex8(rx[i]);
        }
        uart_println("");
    } else {
        uart_println("nothing received");
    }
}