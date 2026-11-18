#include "stm32f446xx.h"
#include "can_types.h"
#include "can.h"

volatile status   g_st_init;
volatile status   g_st_filter;
volatile status   g_st_start;
volatile status   g_rx_count;
volatile status   g_st_rx;


//*****debug helpers**********************************************************************************
static void uart2_init(void){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC->APB1ENR;
    GPIOA->MODER  = (GPIOA->MODER  & ~(3U << 4))   | (2U << 4);   /* PA2 alternate function */
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(0xFU << 8)) | (7U << 8);   /* AF7 = USART2 */
    USART2->BRR = 139;                                            /* 16 MHz / 115200 */
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE;
}

static void uart2_putc(char c){
    while (!(USART2->SR & USART_SR_TXE)) { }
    USART2->DR = (uint8_t)c;
}

static void uart2_puts(const char *s){ while (*s) uart2_putc(*s++); }

static void uart2_hex(uint32_t v, uint8_t digits){
    static const char h[] = "0123456789ABCDEF";
    for (int8_t i = (int8_t)digits - 1; i >= 0; i--) uart2_putc(h[(v >> (4 * i)) & 0xFU]);
}
//************************************************************************************************** 

static void delay_loop(volatile uint32_t n){ while (n--) { } }

int main(void){
    can_config_t abc = {1,1,16000000,500000,625,0,true,true,false,false,false,false,false};

    uart2_init() ; // to print debug info 

    g_st_init = can_init(&abc);
    if (g_st_init != OK) { while(1){} }

    g_st_filter = can_filter_accept_all(0);
    if (g_st_filter != OK) { while(1){} }

    g_st_start = can_start();
    if (g_st_start != OK) { while(1){} }

    frame_t rx ;


    while (1) {
        if (can_rx_pending(0) != 0U) {
            g_st_rx = can_receive(0, &rx);
            if (g_st_rx == OK) {
                g_rx_count++;
                uart2_puts("RX id=0x");  uart2_hex(rx.ID, 3);
                uart2_puts(" len=");     uart2_putc((char)('0' + rx.len));
                uart2_puts(" data:");
                for (uint8_t i = 0; i < rx.len; i++) {
                    uart2_putc(' ');
                    uart2_hex(rx.data[i], 2);
                }
                uart2_puts("\r\n");
            }
        }
    }
}