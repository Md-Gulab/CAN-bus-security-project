#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "can_types.h" 
#include "can.h"
#include "stm32f446xx.h"


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







