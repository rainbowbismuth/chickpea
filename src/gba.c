#include "chickpea.h"

void halt()
{
	__asm__ volatile("swi 0x2");
}

#define BG_PALETTE_RAM 0x05000000
#define VRAM_BEGIN     0x06000000

volatile struct palette *bg_palette(uint32_t palette_idx)
{
	return (volatile struct palette *)(size_t)(BG_PALETTE_RAM +
						   palette_idx * 0x20);
}

volatile struct character_4bpp *character_block_begin(uint32_t char_block)
{
	return (volatile struct character_4bpp *)(size_t)(VRAM_BEGIN +
							  char_block * 0x4000);
}

volatile uint16_t *screen_block_begin(uint32_t screen_block)
{
	return (volatile uint16_t *)(size_t)(VRAM_BEGIN + screen_block * 0x800);
}

volatile uint16_t *reg_bg_control(enum background bg)
{
	return (volatile uint16_t *)(size_t)(0x04000008 + (bg << 1));
}

void set_bg_scroll_x(enum background bg, uint16_t scroll_x)
{
	*(volatile uint16_t *)(size_t)(0x04000010 + (bg << 2)) = scroll_x;
}

void set_bg_scroll_y(enum background bg, uint16_t scroll_y)
{
	*(volatile uint16_t *)(size_t)(0x04000012 + (bg << 2)) = scroll_y;
}

#define MGBA_DEBUG_ENABLE	((volatile uint16_t *)0x04fff780)
#define MGBA_DEBUG_OUTPUT_BEGIN ((volatile char *)0x04fff600)
#define MGBA_DEBUG_OUTPUT_END	((volatile char *)0x04fff700)
#define MGBA_DEBUG_SEND		((volatile uint16_t *)0x04fff700)

volatile char *mgba_debug_output = MGBA_DEBUG_OUTPUT_BEGIN;

void debug_putchar(char c)
{
	if (mgba_debug_output >= MGBA_DEBUG_OUTPUT_END) {
		mgba_debug_output = MGBA_DEBUG_OUTPUT_BEGIN;
		*MGBA_DEBUG_SEND = 0x104;
	}
	if (c == '\0' || c == '\n') {
		mgba_debug_output = MGBA_DEBUG_OUTPUT_BEGIN;
		*MGBA_DEBUG_SEND = 0x104;
		return;
	}
	*mgba_debug_output++ = c;
}

void debug_putstr(const char *str)
{
	do {
		debug_putchar(*str);
	} while (*str++ != '\0');
	debug_putchar('\0');
}

int main(void)
{
	*MGBA_DEBUG_ENABLE = 0xc0de;
	debug_putstr("Hello, mGBA!");
	game_main();
}