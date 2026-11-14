#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "can_types.h" 
#include "can.h"
#include "stm32f446xx.h"
#include "can_timing.h"
#include "can_util.h"


static CAN_TypeDef *s_can = NULL ;

// Put one pin in alternate-function mode.  pupd: 0 none, 1 pull-up, 2 pull-down 
static void can_gpio_af(GPIO_TypeDef *port, uint32_t pin, uint32_t af, uint32_t pupd)
{
    uint32_t sh2 = pin * 2U;
    uint32_t sh4 = (pin & 7U) * 4U;

    port->MODER   = (port->MODER   & ~(3U << sh2)) | (2U   << sh2);  /* AF mode   */
    port->OTYPER &= ~(1U << pin);                                     /* push-pull */
    port->OSPEEDR = (port->OSPEEDR & ~(3U << sh2)) | (2U   << sh2);   /* fast      */
    port->PUPDR   = (port->PUPDR   & ~(3U << sh2)) | (pupd << sh2);
    port->AFR[pin >> 3] = (port->AFR[pin >> 3] & ~(0xFU << sh4)) | (af << sh4);
}

status can_init(const can_config_t *cfg) 
{
    can_bit_timing_t t;
    status    st;

    // 1. validate and compute timing before touching hardware 
    if ((cfg == NULL) || (cfg->instance != CAN_INSTANCE_1)) {
        return INVALID_PARAM;
    }

    st = can_calc_bit_timing(cfg->pclk1_hz, cfg->bitrate, cfg->sample_permille, &t);
    if (st != OK) {
        return st;
    }

    // 2. select peripheral 
    s_can = CAN1 ; 
    
    // 3. clocks 
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
    if (cfg->pins == CAN_PINS_PA11_PA12) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    } else {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    }
    (void)RCC->APB1ENR;     // read-back = short delay 

    // 4. pins (AF9); RX pulled up so a disconnected line reads recessive 
    if (cfg->pins == CAN_PINS_PA11_PA12) {
        can_gpio_af(GPIOA, 11U, 9U, 1U);            // RX 
        can_gpio_af(GPIOA, 12U, 9U, 0U);            // TX 
    } else {
        can_gpio_af(GPIOB, 8U, 9U, 1U);             // RX 
        can_gpio_af(GPIOB, 9U, 9U, 0U);             // TX 
    }

    // 5. master reset: known register state, peripheral ends up in sleep 
    s_can->MCR |= CAN_MCR_RESET;
    st = can_wait_flag(&s_can->MCR, CAN_MCR_RESET, false, CAN_WAIT_TIMEOUT_ITER);
    if (st != OK) { s_can = NULL; return st; }

    // 6. leave sleep, then request init mode 
    s_can->MCR &= ~CAN_MCR_SLEEP;
    st = can_wait_flag(&s_can->MSR, CAN_MSR_SLAK, false, CAN_WAIT_TIMEOUT_ITER);
    if (st != OK) { s_can = NULL; return st; }

    s_can->MCR |= CAN_MCR_INRQ;
    st = can_wait_flag(&s_can->MSR, CAN_MSR_INAK, true, CAN_WAIT_TIMEOUT_ITER);
    if (st != OK) { s_can = NULL; return st; }

    // 7. MCR option bits (read-modify-write keeps INRQ set) 
    uint32_t mcr = s_can->MCR;
    mcr &= ~(CAN_MCR_TTCM | CAN_MCR_ABOM | CAN_MCR_AWUM |
             CAN_MCR_NART | CAN_MCR_RFLM | CAN_MCR_TXFP | CAN_MCR_DBF);
    if (cfg->time_triggered)        { mcr |= CAN_MCR_TTCM; }
    if (cfg->auto_bus_off_recovery) { mcr |= CAN_MCR_ABOM; }
    if (cfg->auto_wakeup)           { mcr |= CAN_MCR_AWUM; }
    if (!cfg->auto_retransmit)      { mcr |= CAN_MCR_NART; }   // inverted logic 
    if (cfg->rx_fifo_locked)        { mcr |= CAN_MCR_RFLM; }
    if (cfg->tx_by_request_order)   { mcr |= CAN_MCR_TXFP; }
    if (cfg->debug_freeze)          { mcr |= CAN_MCR_DBF;  }
    s_can->MCR = mcr;

    // 8. bit timing + test mode, written as one assignment 
    uint32_t btr = ((uint32_t)t.brp << CAN_BTR_BRP_Pos) |
                   ((uint32_t)t.ts1 << CAN_BTR_TS1_Pos) |
                   ((uint32_t)t.ts2 << CAN_BTR_TS2_Pos) |
                   ((uint32_t)t.sjw << CAN_BTR_SJW_Pos);
    if ((cfg->mode == CAN_MODE_LOOPBACK) || (cfg->mode == CAN_MODE_SILENT_LOOPBACK)) {
        btr |= CAN_BTR_LBKM;
    }
    if ((cfg->mode == CAN_MODE_SILENT) || (cfg->mode == CAN_MODE_SILENT_LOOPBACK)) {
        btr |= CAN_BTR_SILM;
    }
    s_can->BTR = btr;

    return OK;      // stays in init mode; can_start() leaves it 
}                            







