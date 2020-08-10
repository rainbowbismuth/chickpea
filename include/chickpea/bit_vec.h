#ifndef CHICKPEA_BIT_VEC_H
#define CHICKPEA_BIT_VEC_H

#include "chickpea/common.h"

void bit_vec_set(uint8_t *nonnull vec, size_t length, size_t n);
void bit_vec_clear(uint8_t *nonnull vec, size_t length, size_t n);
void bit_vec_toggle(uint8_t *nonnull vec, size_t length, size_t n);
bool bit_vec_test(uint8_t *nonnull vec, size_t length, size_t n);

#define BIT_ARRAY_SET(arr, n)	 bit_vec_set(arr, ARRAY_SIZE(arr), n)
#define BIT_ARRAY_CLEAR(arr, n)	 bit_vec_clear(arr, ARRAY_SIZE(arr), n)
#define BIT_ARRAY_TOGGLE(arr, n) bit_vec_toggle(arr, ARRAY_SIZE(arr), n)
#define BIT_ARRAY_TEST(arr, n)	 bit_vec_test(arr, ARRAY_SIZE(arr), n)

#endif // CHICKPEA_BIT_VEC_H