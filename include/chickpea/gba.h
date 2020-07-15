#ifndef CHICKPEA_GBA_H
#define CHICKPEA_GBA_H

#include "stdint.h"
#include "stddef.h"
#include "chickpea/common.h"

// "assert.h" isn't available free-standing, so we define our own.
#define static_assert _Static_assert

/*
 * We define our own assert() macro as well, that as of this moment just
 * infinite loops.
 */
#ifndef NDEBUG
#define assert(cond)                                                           \
	do {                                                                   \
		if ((cond) == 0) {                                             \
			while (1) {                                            \
			}                                                      \
		}                                                              \
	} while (0)
#else
#define assert(cond) (0)
#endif

#define REG_DISPCNT  (*((volatile uint16_t *)0x04000000))
#define REG_DISPSTAT (*((volatile uint16_t *)0x04000004))
#define REG_VCOUNT   (*((volatile uint16_t *)0x04000006))
#define REG_IME	     (*((volatile uint32_t *)0x04000208))
#define REG_IE	     (*((volatile uint16_t *)0x04000200))
#define REG_IF	     (*((volatile uint16_t *)0x04000202))

#define VRAM_BEGIN (0x06000000)

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

static volatile uint32_t *character_block_begin(uint32_t char_block)
{
	return (volatile uint32_t *)(VRAM_BEGIN + char_block * 0x4000);
}

static volatile uint16_t *screen_block_begin(uint32_t screen_block)
{
	return (volatile uint16_t *)(VRAM_BEGIN + screen_block * 0x800);
}

static volatile uint16_t *reg_bg_control(enum background bg)
{
	return (volatile uint16_t *)(0x04000008 + (bg << 2));
}

static volatile uint16_t *reg_bg_scroll_x(enum background bg)
{
	return (volatile uint16_t *)(0x04000010 + (bg << 2));
}

static volatile uint16_t *reg_bg_scroll_y(enum background bg)
{
	return (volatile uint16_t *)(0x04000012 + (bg << 2));
}

#endif //CHICKPEA_GBA_H