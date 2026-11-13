#ifndef CAN_UTIL_H
#define CAN_UTIL_H

#include <stdint.h>
#include <stdbool.h>
#include "can_types.h"

status can_wait_flag(volatile uint32_t *reg,
                           uint32_t mask,
                           bool expect_set,
                           uint32_t timeout);

#endif