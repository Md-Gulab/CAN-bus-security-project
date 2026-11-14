/* test_can_timing.c */
#include "unity.h"
#include "can_types.h"
#include "can_timing.h"
#include "can_util.h"

void setUp(void)    { }     /* runs before each test; Unity requires it to exist */
void tearDown(void) { }     /* runs after each test; same */

/* Helper to avoid repeating five asserts per case */
static void check_timing(uint32_t pclk, uint32_t bitrate, uint16_t sp,
                         uint16_t brp, uint8_t ts1, uint8_t ts2,
                         uint8_t sjw, uint16_t sample)
{
    can_bit_timing_t t;
    status st = can_calc_bit_timing(pclk, bitrate, sp, &t);

    TEST_ASSERT_EQUAL(OK, st);
    TEST_ASSERT_EQUAL_UINT16(brp,    t.brp);
    TEST_ASSERT_EQUAL_UINT8 (ts1,    t.ts1);
    TEST_ASSERT_EQUAL_UINT8 (ts2,    t.ts2);
    TEST_ASSERT_EQUAL_UINT8 (sjw,    t.sjw);
    TEST_ASSERT_EQUAL_UINT16(sample, t.sample_permille);
}

void test_16MHz_500k_sp875(void)
{
    check_timing(16000000UL, 500000UL, 875, 1, 12, 1, 1, 875);
}

void test_16MHz_500k_sp625(void)
{
    check_timing(16000000UL, 500000UL, 625, 1, 8, 5, 3, 625);
}

void test_16MHz_1M_sp875(void)
{
    check_timing(16000000UL, 1000000UL, 875, 0, 12, 1, 1, 875);
}

void test_45MHz_500k_sp875(void)
{
    check_timing(45000000UL, 500000UL, 875, 5, 11, 1, 1, 867);
}

void test_impossible_bitrate_returns_invalid(void)
{
    can_bit_timing_t t;
    TEST_ASSERT_EQUAL(INVALID_PARAM,
                      can_calc_bit_timing(16000000UL, 3000000UL, 875, &t));
}

void test_null_output_returns_invalid(void)
{
    TEST_ASSERT_EQUAL(INVALID_PARAM,
                      can_calc_bit_timing(16000000UL, 500000UL, 875, NULL));
}

void test_zero_bitrate_returns_invalid(void)
{
    can_bit_timing_t t;
    TEST_ASSERT_EQUAL(INVALID_PARAM,
                      can_calc_bit_timing(16000000UL, 0UL, 875, &t));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_16MHz_500k_sp875);
    RUN_TEST(test_16MHz_500k_sp625);
    RUN_TEST(test_16MHz_1M_sp875);
    RUN_TEST(test_45MHz_500k_sp875);
    RUN_TEST(test_impossible_bitrate_returns_invalid);
    RUN_TEST(test_null_output_returns_invalid);
    RUN_TEST(test_zero_bitrate_returns_invalid);
    return UNITY_END();
}