#ifndef CAN_TIMING_H
#define CAN_TIMING_H

#include <stdint.h>
#include "can_types.h"

typedef struct {
    uint16_t brp;
    uint8_t  ts1;
    uint8_t  ts2;
    uint8_t  sjw;
    uint16_t sample_permille;
} can_bit_timing_t;

status can_calc_bit_timing(uint32_t pclk_hz,
                                 uint32_t bitrate,
                                 uint16_t sample_permille,
                                 can_bit_timing_t *out);

#endif