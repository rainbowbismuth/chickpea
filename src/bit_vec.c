#include "chickpea.h"
#include "chickpea/bit_vec.h"
#include "chickpea/nano_unit.h"

void bit_vec_set(uint8_t *nonnull vec, size_t length, size_t n)
{
	assert(n < length * 8);
	vec[n / 8] |= 1 << (n % 8);
}

void bit_vec_clear(uint8_t *nonnull vec, size_t length, size_t n)
{
	assert(n < length * 8);
	vec[n / 8] &= ~(1 << (n % 8));
}

bool bit_vec_test(uint8_t *nonnull vec, size_t length, size_t n)
{
	assert(n < length * 8);
	return (vec[n / 8] & (1 << (n % 8))) != 0;
}

static void test_set(struct nano_unit_case *nonnull test)
{
	uint8_t bytes[5] = {};
	bit_vec_set(bytes, ARRAY_SIZE(bytes), 7);
	bit_vec_set(bytes, ARRAY_SIZE(bytes), 8);
	bit_vec_set(bytes, ARRAY_SIZE(bytes), 9);

	NANO_ASSERT(test, bytes[0] == 0x80, exit);
	NANO_ASSERT(test, bytes[1] == 3, exit);
	NANO_ASSERT(test, bytes[2] == 0, exit);
exit:
	return;
}

static void test_clear_and_test(struct nano_unit_case *nonnull test)
{
	uint8_t bytes[5] = {};
	size_t indices[7] = { 1, 3, 9, 10, 20, 21, 31 };
	for (size_t i = 0; i < ARRAY_SIZE(indices); ++i) {
		bit_vec_set(bytes, ARRAY_SIZE(bytes), indices[i]);
	}
	for (size_t i = 0; i < ARRAY_SIZE(indices); ++i) {
		NANO_ASSERT(test,
			    bit_vec_test(bytes, ARRAY_SIZE(bytes), indices[i]),
			    exit);
	}
	for (size_t i = 0; i < ARRAY_SIZE(indices); ++i) {
		bit_vec_clear(bytes, ARRAY_SIZE(bytes), indices[i]);
	}
	for (size_t i = 0; i < ARRAY_SIZE(indices); ++i) {
		NANO_ASSERT(test,
			    !bit_vec_test(bytes, ARRAY_SIZE(bytes), indices[i]),
			    exit);
	}
exit:
	return;
}

struct nano_unit_case bit_vec_test_suite[] = { NANO_UNIT_CASE(test_set),
					       NANO_UNIT_CASE(
						       test_clear_and_test),
					       {} };