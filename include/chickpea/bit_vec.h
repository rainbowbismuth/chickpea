#ifndef CHICKPEA_BIT_VEC_H
#define CHICKPEA_BIT_VEC_H

#include "chickpea/common.h"

void bit_vec_set(uint8_t *nonnull vec, size_t length, size_t n);
void bit_vec_clear(uint8_t *nonnull vec, size_t length, size_t n);
bool bit_vec_test(uint8_t *nonnull vec, size_t length, size_t n);

#endif //CHICKPEA_BIT_VEC_H