#ifndef CAN_H
#define CAN_H

#include<stdint.h>
#include<stdbool.h>
#include "can_types.h"
#include "stm32f446xx.h"



typedef struct CONFIG{
    uint8_t instance        ; // 1 0r 2
    uint8_t pinset          ; // pin number
    uint16_t bitrate        ; // speed on CAN bus
    uint8_t sample_point    ; // when to sample
    uint8_t mode            ; // normal, loopback, silent, s+lb
    uint8_t auto_retransmit ; // 1 or 0
    uint8_t auto_busRecovery; // on or off
    uint8_t Tx_Order        ; // by ID or by request
    uint8_t Rx_FIFO_Config  ; // locked or overwrite
    uint8_t Hw_timestamps   ; // on or off 
}config;

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

typedef struct {
    uint16_t brp;               // register value = prescaler - 1   (0..1023) 
    uint8_t  ts1;               // register value = tBS1 - 1        (0..15)   
    uint8_t  ts2;               // register value = tBS2 - 1        (0..7)    
    uint8_t  sjw;               // register value = SJW - 1         (0..3)    
    uint16_t sample_permille;   // sample point actually achieved   
} can_bit_timing_t;

                                 
static void can_gpio_af(GPIO_TypeDef *port, uint32_t pin, uint32_t af, uint32_t pupd) ;                                 



#endif