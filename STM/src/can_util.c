#include "can_util.h"


status can_wait_flag(volatile uint32_t *reg,
                                  uint32_t mask,
                                  bool expect_set,
                                  uint32_t timeout)
{
    if ((reg == NULL) || (mask == 0U)) {
        return INVALID_PARAM;
    }

    do {
        uint32_t bits = (*reg) & mask;

        if (expect_set ? (bits == mask) : (bits == 0U)) {
            return OK;
        }
    } while (timeout-- > 0U);

    return TIMEOUT;
}