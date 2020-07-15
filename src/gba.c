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
	return (volatile uint16_t *)(size_t)(0x04000008 + (bg << 2));
}

volatile uint16_t *reg_bg_scroll_x(enum background bg)
{
	return (volatile uint16_t *)(size_t)(0x04000010 + (bg << 2));
}

volatile uint16_t *reg_bg_scroll_y(enum background bg)
{
	return (volatile uint16_t *)(size_t)(0x04000012 + (bg << 2));
}

int main(void)
{
	game_main();
}