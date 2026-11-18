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

    mcp2515_reset();                                   // also clears any stuck transmit request
    mcp2515_set_bitrate_500k_8mhz();
    mcp2515_write_reg(MCP_RXB0CTRL, 0x60);             // accept all frames
    mcp2515_set_mode(MCP_MODE_NORMAL);
    // uart_print("CANSTAT="); uart_print_hex8(mcp2515_read_reg(0x0E));
    // uart_print(" CNF1=");   uart_print_hex8(mcp2515_read_reg(0x2A));
    // uart_print(" CNF2=");   uart_print_hex8(mcp2515_read_reg(0x29));
    // uart_print(" CNF3=");   uart_print_hex8(mcp2515_read_reg(0x28));
    // uart_println("");

    // uart_println("listening...");

    //uint16_t id; uint8_t rx[8]; uint8_t len;

    uint8_t tx[4] = { 0x00, 0xBB, 0xCC, 0xDD };
    uint8_t count = 0;

    while (1) {
        tx[0] = count++;                         // changes every frame
        mcp2515_send_std(0x321, tx, 4);

        uart_print("TX id=0x321 len=4 data:");
        for (uint8_t i = 0; i < 4; i++) {
            usart_write(' ');
            uart_print_hex8(tx[i]);
        }
        uart_println("");

        _delay_ms(500);
    }
}