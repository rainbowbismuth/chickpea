#ifndef CHICKPEA_GBA_H
#define CHICKPEA_GBA_H

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

#define BG_PALETTE_RAM ((volatile uint16_t *)0x05000000)

#define VRAM_BEGIN (0x06000000)

static volatile uint32_t *character_block_begin(uint32_t char_block)
{
	return (volatile uint32_t *)(size_t)(VRAM_BEGIN + char_block * 0x4000);
}

static volatile uint16_t *screen_block_begin(uint32_t screen_block)
{
	return (volatile uint16_t *)(size_t)(VRAM_BEGIN + screen_block * 0x800);
}

static volatile uint16_t *reg_bg_control(enum background bg)
{
	return (volatile uint16_t *)(size_t)(0x04000008 + (bg << 2));
}

static volatile uint16_t *reg_bg_scroll_x(enum background bg)
{
	return (volatile uint16_t *)(size_t)(0x04000010 + (bg << 2));
}

static volatile uint16_t *reg_bg_scroll_y(enum background bg)
{
	return (volatile uint16_t *)(size_t)(0x04000012 + (bg << 2));
}

#endif //CHICKPEA_GBA_H