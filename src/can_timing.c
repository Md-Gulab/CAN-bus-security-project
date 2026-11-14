#include "can_timing.h"
#include <stdbool.h>



#define CAN_TQ_MIN      8U      // fewest time quanta per bit we will use 
#define CAN_TQ_MAX      25U     // hardware maximum: 1 + 16 + 8           
#define CAN_BRP_MAX     1024U
#define CAN_TBS1_MAX    16U
#define CAN_TBS2_MAX    8U
#define CAN_SJW_MAX     4U


status can_calc_bit_timing(uint32_t pclk_hz,
                                 uint32_t bitrate,
                                 uint16_t sample_permille,
                                 can_bit_timing_t *out)
{
    if ((out == NULL) || (pclk_hz == 0U) || (bitrate == 0U) ||
        (sample_permille == 0U) || (sample_permille >= 1000U)) {
        return INVALID_PARAM;
    }

    bool             found     = false;
    uint32_t         best_diff = UINT32_MAX;
    can_bit_timing_t best      = {0};

    for (uint32_t total = CAN_TQ_MAX; total >= CAN_TQ_MIN; total--) {

        // Bitrate must come out exact: pclk = bitrate * total * prescaler
        uint64_t denom = (uint64_t)bitrate * total;
        if ((pclk_hz % denom) != 0U) {
            continue;
        }
        uint32_t presc = (uint32_t)(pclk_hz / denom);
        if ((presc < 1U) || (presc > CAN_BRP_MAX)) {
            continue;
        }

        // sample point = (1 + tBS1) / total, so 1 + tBS1 = round(total * sp) 
        uint32_t sync_plus_tbs1 = (total * sample_permille + 500U) / 1000U;
        if ((sync_plus_tbs1 < 2U) || (sync_plus_tbs1 >= total)) {
            continue;
        }
        uint32_t tbs1 = sync_plus_tbs1 - 1U;
        uint32_t tbs2 = total - sync_plus_tbs1;

        if ((tbs1 < 1U) || (tbs1 > CAN_TBS1_MAX)) { continue; }
        if ((tbs2 < 1U) || (tbs2 > CAN_TBS2_MAX)) { continue; }

        uint32_t actual = (sync_plus_tbs1 * 1000U + total / 2U) / total;
        uint32_t diff   = (actual > sample_permille)
                              ? (actual - sample_permille)
                              : (sample_permille - actual);

        // Strict '<' plus a descending loop means ties keep the larger total 
        if (diff < best_diff) {
            best_diff = diff;

            uint32_t sjw = (tbs2 < CAN_SJW_MAX) ? tbs2 : CAN_SJW_MAX;

            best.brp             = (uint16_t)(presc - 1U);
            best.ts1             = (uint8_t)(tbs1 - 1U);
            best.ts2             = (uint8_t)(tbs2 - 1U);
            best.sjw             = (uint8_t)(sjw - 1U);
            best.sample_permille = (uint16_t)actual;
            found = true;
        }
    }

    if (!found) {
        return INVALID_PARAM;
    }

    *out = best;
    return OK;
}