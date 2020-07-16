#include "stdint.h"
#include "stddef.h"

#include "chickpea.h"
#include "chickpea/debug_font.h"

void our_irq_handler(void)
{
	;
}

void (*volatile irq_handler)(void) = our_irq_handler;

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

void game_main(void)
{
	REG_DISPCNT = DISPCNT_FORCED_BLANK | DISPCNT_SCREEN_DISPLAY_BG0 |
		      DISPCNT_SCREEN_DISPLAY_BG1;

	bg_palette(0)->color[0] = color(10, 5, 31);

	*reg_bg_control(BG0) = PREP(BGCNT_SCREEN_BLOCK, 1) |
			       PREP(BGCNT_PRIORITY, 1);

	write_4bpp(&demo_tile, &character_block_begin(0)[1]);

	*reg_bg_control(BG1) = PREP(BGCNT_CHAR_BLOCK, 2) |
			       PREP(BGCNT_SCREEN_BLOCK, 2);

	volatile uint16_t *tiles = screen_block_begin(1);
	for (size_t i = 0; i < 16 * 32; ++i) {
		tiles[i << 1] = PREP(TILE_CHAR, 1) | PREP(TILE_PALETTE, i % 4);
	}

	write_debug_msg(&default_debug_font, 2, 2, 4, 3, 3, "Hello, world!");

	REG_BLDCNT = BLDCNT_1ST_TARGET_BG1 | BLDCNT_2ND_TARGET_BG0 |
		     BLDCNT_2ND_TARGET_BD | PREP(BLDCNT_EFFECT, BLEND_ALPHA);
	REG_BLDALPHA = PREP(BLDALPHA_1ST_WEIGHT, 8) |
		       PREP(BLDALPHA_2ND_WEIGHT, 8);

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
