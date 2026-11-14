#ifndef CAN_H
#define CAN_H

#include<stdint.h>
#include<stdbool.h>
#include "can_types.h"
#include "stm32f446xx.h"



typedef enum { CAN_INSTANCE_1 = 1 } can_instance_t;      // CAN2 later 

typedef enum {
    CAN_PINS_PA11_PA12 = 0,     // RX = PA11, TX = PA12 
    CAN_PINS_PB8_PB9   = 1      // RX = PB8,  TX = PB9  
} can_pinset_t;

typedef enum {
    CAN_MODE_NORMAL = 0,
    CAN_MODE_LOOPBACK,
    CAN_MODE_SILENT,
    CAN_MODE_SILENT_LOOPBACK
} can_mode_t;

typedef struct {
    can_instance_t instance;
    can_pinset_t   pins;
    uint32_t       pclk1_hz;            // actual APB1 clock 
    uint32_t       bitrate;
    uint16_t       sample_permille;     // e.g. 875 
    can_mode_t     mode;
    bool           auto_retransmit;     // true  -> NART = 0 
    bool           auto_bus_off_recovery;   // ABOM 
    bool           tx_by_request_order;     // TXFP 
    bool           rx_fifo_locked;          // RFLM 
    bool           time_triggered;          // TTCM 
    bool           auto_wakeup;             // AWUM 
    bool           debug_freeze;            // DBF 
} can_config_t;

typedef struct  FRAME{
    uint32_t ID             ; // frame ID
    uint8_t ID_type         ; // standard or extended
    uint8_t frame_type      ; // data or remote
    uint8_t frame_length    ; // 0-8
    uint8_t data[8]         ; // actual data
    uint8_t time_stamps     ; // the time frame was sent
    uint8_t matched_fil_idx ; // index of filter
    uint8_t FIFO_number     ; // 0-x(8)
}frame;

typedef struct FILTER_CONFIG{ // bank number, scale (16/32-bit), mode (mask/list), FIFO assignment, ID1, ID2 or mask, ID type, frame type, enable flag
    uint8_t bank_no         ; // which bank ?
    uint8_t scale           ; // 16 or 32
    uint8_t mode            ; // mask or list
    uint8_t FIFO_no         ; // 0 or 1
    uint32_t ID1            ; // 11 bits or 29 if etended
    uint32_t ID2_mask       ; // list mode & mask mode
    uint8_t  ide            ; // true = extended ID, false = standard ID (applies to id1, and id2 in list mode) 
    uint8_t rtr             ; // true = remote frame, false = data frame (same scope as ide) 
    uint8_t mask_match_ide  ; // mask mode only: true = IDE bit must match, false = ignore it 
    uint8_t mask_match_rtr  ; // mask mode only: true = RTR bit must match, false = ignore it 
    uint8_t enable          ; // activate the bank after configuring it             
}filter_config;

typedef struct STATS{
    bool frame_sent             ; // return 1 for success
    bool frame_received         ; // return 1 for success
    bool frame_dropped          ; // frame dropped due to bufffer full ?? return 1 
    uint8_t error_count_per_type; // count of error
    uint8_t bus_off_count       ; // count of bus-off
}stats;

                                 
static void can_gpio_af(GPIO_TypeDef *port, uint32_t pin, uint32_t af, uint32_t pupd) ;  
status can_init(const can_config_t *cfg) ;                               



#endif