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

    port->MODER   = (port->MODER   & ~(3U << sh2)) | (2U   << sh2);   // AF mode   
    port->OTYPER &= ~(1U << pin);                                     // push-pull 
    port->OSPEEDR = (port->OSPEEDR & ~(3U << sh2)) | (2U   << sh2);   // fast      
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

status can_start(void){
    if(s_can==NULL) return CAN_ERR_NOT_INITIALISED ;
    s_can->MCR &= (0xfffffffe) ;
    status st = can_wait_flag(&s_can->MSR,                  // which register to watch  
                                CAN_MSR_INAK,               // which bit to watch       
                                false,                      // wait until it becomes 0  
                                CAN_WAIT_TIMEOUT_ITER);

    return st; 
}

status can_filter_accept_all(uint8_t fifo){
    if(fifo > 1) return INVALID_PARAM ;
    if(s_can==NULL) return CAN_ERR_NOT_INITIALISED ;


    s_can->FMR |= (1U) ;
    s_can->FA1R &= (0xfffffffe) ;
    s_can->FS1R |= (1U) ;
    s_can->FM1R &= (0xfffffffe) ;

    if(fifo) s_can->FFA1R |= (1U) ;
    else s_can->FFA1R &= (0xfffffffe) ;
    s_can->sFilterRegister[0].FR1 = 0 ;
    s_can->sFilterRegister[0].FR2 = 0 ;
    
    s_can->FA1R |= (1U) ;
    s_can->FMR &= (0xfffffffe) ;

    return OK ;

}

status can_transmit(const frame_t *frame){
    if(s_can==NULL) return CAN_ERR_NOT_INITIALISED ;
    if(frame==NULL) return INVALID_PARAM ;
    if(frame->len > 8) return INVALID_LEN ;
    

    for(int mb=0 ; mb<3 ; mb++){
        if(s_can->TSR & (1U<<(26+mb))){
            if(frame->extended) s_can->sTxMailBox[mb].TIR = (frame->ID << 3) | (1U<<2) ;
            else s_can->sTxMailBox[mb].TIR = (frame->ID << 21) ;

            if(frame->remote) s_can->sTxMailBox[mb].TIR |= (2U) ;
            
            s_can->sTxMailBox[mb].TDTR = frame->len ;
            uint8_t j = 0 ;
            s_can->sTxMailBox[mb].TDLR = 0 ;
            s_can->sTxMailBox[mb].TDHR = 0 ;
            for(uint8_t i=0 ; i<(frame->len) ; i++){
                if(i<=3){
                    s_can->sTxMailBox[mb].TDLR |=  (uint32_t)((frame->data[i])  << (8*i)) ;
                } 
                else{ 
                    s_can->sTxMailBox[mb].TDHR |= (uint32_t)((frame->data[i])  << (8*j)) ;
                    j++ ;
                }        
            }
            s_can->sTxMailBox[mb].TIR |= 1U ;   
        }
        return OK ;
    }
    return CAN_ERR_NO_FREE_MAILBOX ;
}

uint8_t can_rx_pending(uint8_t fifo){
    if (s_can == NULL) return 0U;

    if (fifo == 0U) {
        return (uint8_t)(s_can->RF0R & CAN_RF0R_FMP0);    // low 2 bits: 0..3 
    } else if (fifo == 1U) {
        return (uint8_t)(s_can->RF1R & CAN_RF1R_FMP1);
    }
    return 0U;                                            // invalid fifo number 
}

uint8_t can_tx_free_mailboxes(void){
    if (s_can == NULL) return 0U;

    uint32_t tsr  = s_can->TSR;                           // read once 
    uint8_t  free = 0U;

    if (tsr & CAN_TSR_TME0) free++;
    if (tsr & CAN_TSR_TME1) free++;
    if (tsr & CAN_TSR_TME2) free++;

    return free;                                          // 0..3 
}

status can_receive(uint8_t fifo, frame_t *out){
    if(s_can==NULL) return CAN_ERR_NOT_INITIALISED ;
    if(out==NULL || fifo >1) return INVALID_PARAM ;
    
    if(!can_rx_pending(fifo)) return CAN_ERR_FIFO_EMPTY ;

    if(s_can->sFIFOMailBox[fifo].RIR & (4U)){
        out->extended = true ;
        out->ID = (s_can->sFIFOMailBox[fifo].RIR) >> 3 ;
    } 
    else{
        out->extended = false ;
        out->ID = (s_can->sFIFOMailBox[fifo].RIR) >> 21 ;
    } 

    out->remote = ((s_can->sFIFOMailBox[fifo].RIR& 2U) != 0U) ; 


    out->len = (s_can->sFIFOMailBox[fifo].RDTR) & (0xF) ;
    if(out->len > 8 ) out->len = 8 ;

    out->FIFO_number = fifo ;
    out->matched_fil_idx = (s_can->sFIFOMailBox[fifo].RDTR  >> 8) & 0xFF ;
    out->time_stamps = s_can->sFIFOMailBox[fifo].RDTR >> 16 ;

    for(uint8_t i=0 ; i<8 ; i++){
        out->data[i] = 0 ;
    }
    uint8_t j = 0 ;
    for(uint8_t i=0 ; i<(out->len) ; i++){
        if(i<=3){
            out->data[i] =  (s_can->sFIFOMailBox[fifo].RDLR >> (8*i)) & 0xFF ;
        }
        else {
            out->data[i] =  (s_can->sFIFOMailBox[fifo].RDHR >> (8*j)) & 0xFF ; 
            j++ ; 
        }
    }

    if (fifo == 0U) {
        s_can->RF0R = CAN_RF0R_RFOM0 ;
    } else {
        s_can->RF1R = CAN_RF1R_RFOM1 ;
    }

    return OK ;
}







