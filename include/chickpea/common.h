#ifndef CHICKPEA_COMMON_H
#define CHICKPEA_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * Attributes for pointers that declare if they're nullable or not.
 */
#define nonnull	 _Nonnull
#define nullable _Nullable

/*
 * Assert that condition is true at compile time, and evaluates to zero if
 * compilation succeeds.
 */
#define BUILD_CHECK(condition) (sizeof(typeof(int[1 - 2 * !!(condition)])) * 0)

/*
 * The size of the passed in array, with a test to fail compilation if `arr`
 * is a pointer.
 */
#define ARRAY_SIZE(arr)                   \
	(sizeof(arr) / sizeof((arr)[0]) + \
	 BUILD_CHECK(                     \
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
#define FIELD(offset, width) \
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
#define GET(mask, val) (((val)&CONSTANT(mask)) >> MASK_OFFSET(mask))

/*
 * Masks & offsets val so that it is ready to be bitwise or'd with other fields
 */
#define PREP(mask, val) (((val) << MASK_OFFSET(CONSTANT(mask))) & (mask))

/*
 * Replace the field with a new value
 */
#define REPLACE(mask, val, new_val) ((val & ~mask) | PREP(mask, new_val))

#define GBA_WIDTH  240
#define GBA_HEIGHT 160

#define DISPCNT_FORCED_BLANK	   BIT(7)
#define DISPCNT_SCREEN_BG_ENABLED  FIELD(8, 4)
#define DISPCNT_SCREEN_DISPLAY_BG0 BIT(8)
#define DISPCNT_SCREEN_DISPLAY_BG1 BIT(9)
#define DISPCNT_SCREEN_DISPLAY_BG2 BIT(10)
#define DISPCNT_SCREEN_DISPLAY_BG3 BIT(11)

#define DISPSTAT_VERTICAL_BLANK		      BIT(0)
#define DISPSTAT_HORIZONTAL_BLANK	      BIT(1)
#define DISPSTAT_VERTICAL_BLANK_IRQ_ENABLED   BIT(3)
#define DISPSTAT_HORIZONTAL_BLANK_IRQ_ENABLED BIT(4)

#define COL_BLUE  FIELD(10, 5)
#define COL_GREEN FIELD(5, 5)
#define COL_RED	  FIELD(0, 5)

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

#define BLDCNT_1ST_TARGET     FIELD(0, 6)
#define BLDCNT_1ST_TARGET_BG0 BIT(0)
#define BLDCNT_1ST_TARGET_BG1 BIT(1)
#define BLDCNT_1ST_TARGET_BG2 BIT(2)
#define BLDCNT_1ST_TARGET_BG3 BIT(3)
#define BLDCNT_1ST_TARGET_OBJ BIT(4)
#define BLDCNT_1ST_TARGET_BD  BIT(5)
#define BLDCNT_EFFECT	      FIELD(6, 2)
#define BLDCNT_2ND_TARGET     FIELD(8, 6)
#define BLDCNT_2ND_TARGET_BG0 BIT(8)
#define BLDCNT_2ND_TARGET_BG1 BIT(9)
#define BLDCNT_2ND_TARGET_BG2 BIT(10)
#define BLDCNT_2ND_TARGET_BG3 BIT(11)
#define BLDCNT_2ND_TARGET_OBJ BIT(12)
#define BLDCNT_2ND_TARGET_BD  BIT(13)

#define BLDALPHA_1ST_WEIGHT FIELD(0, 4)
#define BLDALPHA_2ND_WEIGHT FIELD(8, 4)

#define BLDY_BRIGHTNESS FIELD(0, 4)

#define KEYINPUT_BUTTON_A BIT(0)
#define KEYINPUT_BUTTON_B BIT(1)
#define KEYINPUT_SELECT	  BIT(2)
#define KEYINPUT_START	  BIT(3)
#define KEYINPUT_RIGHT	  BIT(4)
#define KEYINPUT_LEFT	  BIT(5)
#define KEYINPUT_UP	  BIT(6)
#define KEYINPUT_DOWN	  BIT(7)
#define KEYINPUT_BUTTON_R BIT(8)
#define KEYINPUT_BUTTON_L BIT(9)

/*
 * The GBA doesn't have X or Y buttons, but these bits are unused, so they
 * will be implemented for non-GBA platforms, particularly for debug
 * functionality.
 */
#define KEYINPUT_BUTTON_X BIT(10)
#define KEYINPUT_BUTTON_Y BIT(11)

enum blend_effect {
	BLEND_NONE = 0,
	BLEND_ALPHA = 1,
	BLEND_BRIGHTNESS_INCREASE = 2,
	BLEND_BRIGHTNESS_DECREASE = 3
};

enum background { BG0 = 0, BG1, BG2, BG3 };

struct character_4bpp {
	uint32_t lines[8];
};

struct palette {
	uint16_t color[16];
};

volatile struct character_4bpp *nonnull
character_block_begin(uint32_t char_block);

volatile struct palette *nonnull bg_palette(uint32_t palette_idx);

volatile uint16_t *nonnull screen_block_begin(uint32_t screen_block);

volatile uint16_t *nonnull reg_bg_control(enum background bg);

void set_bg_scroll_x(enum background bg, uint16_t scroll_x);

void set_bg_scroll_y(enum background bg, uint16_t scroll_y);

void halt(void);

uint32_t reverse_nibbles(uint32_t n);

void write_4bpp(const struct character_4bpp *restrict nonnull src,
		volatile struct character_4bpp *restrict nonnull dst);

void write_palette(const struct palette *restrict nonnull src,
		   volatile struct palette *restrict nonnull dst);

void debug_put_char(char c);

void debug_put_str(const char *nonnull str);

void debug_put_u32(uint32_t n);

uint16_t color(uint32_t red, uint32_t green, uint32_t blue);

uint16_t additive_blend(uint16_t src_color, uint16_t src_weight,
			uint16_t dst_color, uint16_t dst_weight);

extern void (*nonnull volatile irq_handler)(void);

#endif //CHICKPEA_COMMON_H