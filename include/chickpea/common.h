#ifndef CHICKPEA_COMMON_H
#define CHICKPEA_COMMON_H

/*
 * Assert that condition is true at compile time, and evaluates to zero if
 * compilation succeeds.
 */
#define BUILD_CHECK(condition) (sizeof(typeof(int[1 - 2 * !!(condition)])) * 0)

/*
 * The size of the passed in array, with a test to fail compilation if `arr`
 * is a pointer.
 */
#define ARRAY_SIZE(arr)                                                        \
	(sizeof(arr) / sizeof((arr)[0]) +                                      \
	 BUILD_CHECK(                                                          \
		 __builtin_types_compatible_p(typeof(arr), typeof(&arr[0]))))

/*
 * Asserts that `expr` is a positive integer constant expression, and returns
 * that same number.
 */
#define CONSTANT(expr) (sizeof(char[expr]))

/*
 * Asserts that `expr` is a positive integer constant expression that is
 * greater than zero (GTZ) and returns that same number.
 */
#define CONSTANT_GTZ(expr) (sizeof(char[(expr)-1]) + 1)

/*
 * Defines a bit mask for a field with multiple bits
 */
#define FIELD(offset, width)                                                   \
	(((1 << CONSTANT_GTZ(width)) - 1) << CONSTANT(offset))

/*
 * Defines a single bit field
 */
#define BIT(offset) (1 << CONSTANT(offset))

/*
 * Returns the offset of the mask such that
 *
 *	MASK_OFFSET(FIELD(offset, width)) == offset
 */
#define MASK_OFFSET(v) (__builtin_ffs(CONSTANT(v)) - 1)

/*
 * Returns the field from val, such that
 *
 *	GET(mask, PREP(mask, val)) == val
 */
#define GET(mask, val) ((val & CONSTANT(mask)) >> MASK_OFFSET(mask))

/*
 * Masks & offsets val so that it is ready to be bitwise or'd with other fields
 */
#define PREP(mask, val) ((val << MASK_OFFSET(CONSTANT(mask))) & mask)

#endif //CHICKPEA_COMMON_H