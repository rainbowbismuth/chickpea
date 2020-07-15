#ifndef CHICKPEA_COMMON_H
#define CHICKPEA_COMMON_H

#include "stdint.h"
#include "stddef.h"

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



#define DISPCNT_FORCED_BLANK	   BIT(7)
#define DISPCNT_SCREEN_DISPLAY_BG0 BIT(8)

#define DISPSTAT_VERTICAL_BLANK_IRQ_ENABLED   BIT(3)
#define DISPSTAT_HORIZONTAL_BLANK_IRQ_ENABLED BIT(4)

#define COL_RED	  FIELD(10, 5)
#define COL_GREEN FIELD(5, 5)
#define COL_BLUE  FIELD(0, 5)

#define TILE_CHAR	     FIELD(0, 9)
#define TILE_HORIZONTAL_FLIP BIT(10)
#define TILE_VERTICAL_FLIP   BIT(11)
#define TILE_PALETTE	     FIELD(12, 4)

#define INT_VERTICAL_BLANK   BIT(0)
#define INT_HORIZONTAL_BLANK BIT(1)

#define BGCNT_PRIORITY	   FIELD(0, 2)
#define BGCNT_CHAR_BLOCK   FIELD(2, 2)
#define BGCNT_MOSAIC	   BIT(6)
#define BGCNT_256_COLORS   BIT(7)
#define BGCNT_SCREEN_BLOCK FIELD(8, 5)
#define BGCNT_SCREEN_SIZE  FIELD(14, 2)

enum background { BG0 = 0, BG1, BG2, BG3 };

void halt(void);


#endif //CHICKPEA_COMMON_H