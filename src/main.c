#include "stdint.h"
#include "stddef.h"

#include "chickpea.h"

void our_irq_handler(void)
{
	;
}

void (*irq_handler)(void) = our_irq_handler;

uint16_t color(uint32_t red, uint32_t green, uint32_t blue)
{
	return PREP(COL_RED, red) | PREP(COL_GREEN, green) |
	       PREP(COL_BLUE, blue);
}

void wait_for_horizontal_blank()
{
	REG_IME = 0;
	REG_DISPSTAT |= DISPSTAT_HORIZONTAL_BLANK_IRQ_ENABLED;
	REG_IE |= INT_HORIZONTAL_BLANK;
	REG_IME = 1;
	halt();
	REG_IME = 0;
}

static const struct character_4bpp
	demo_tile = { .lines = {
			      0b00010001000100010001000100010001u,
			      0b00010000000000000000000000000001u,
			      0b00010000000000010001000000000001u,
			      0b00010000000000010001000000000001u,
			      0b00010000000000010001000000000001u,
			      0b00010000000000010001000000000001u,
			      0b00010000000000000000000000000001u,
			      0b00010001000100010001000100010001u,
		      } };

void write_4bpp(const struct character_4bpp *src,
		volatile struct character_4bpp *dst)
{
	for (size_t i = 0; i < ARRAY_SIZE(src->lines); ++i) {
		dst->lines[i] = src->lines[i];
	}
}

void game_main(void)
{
	REG_DISPCNT = DISPCNT_FORCED_BLANK | DISPCNT_SCREEN_DISPLAY_BG0;
	bg_palette(0)->color[0] = color(10, 5, 31);

	*reg_bg_control(BG0) = PREP(BGCNT_SCREEN_BLOCK, 1);
	write_4bpp(&demo_tile, &character_block_begin(0)[1]);

	volatile uint16_t *screen_block = screen_block_begin(1);
	for (size_t i = 0; i < 16 * 32; ++i) {
		screen_block[i << 1] = PREP(TILE_CHAR, 1) |
				       PREP(TILE_PALETTE, i % 4);
	}

	REG_DISPCNT &= ~DISPCNT_FORCED_BLANK;

	uint32_t frame = 0;
	while (1) {
		wait_for_horizontal_blank();
		bg_palette(0)->color[0] =
			color(((REG_VCOUNT >> 2) & 0b1111), 5, 10);

		if (REG_VCOUNT == 160) {
			frame++;
			uint32_t c = (frame >> 2) & 0x1F;
			bg_palette(0)->color[1] = color(22, c, 15);
			bg_palette(1)->color[1] = color(c, 22, 15);
			bg_palette(2)->color[1] = color(15, 22, c);
			bg_palette(3)->color[1] = color(c, c, c);

			uint32_t v = (frame >> 1) % 512;

			*reg_bg_scroll_x(BG0) = v;
			*reg_bg_scroll_y(BG0) = v;
		}
	}
}
