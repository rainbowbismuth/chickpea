#include "game/text_box.h"

#define BOX_GFX_WIDTH	 4
#define BOX_GFX_WIDTH_M1 (BOX_GFX_WIDTH - 1)

void text_box_draw(const struct text_box_settings *settings)
{
	// FIXME: Hard coded to be first in block..?
	volatile struct char_4bpp *out = char_block_begin(settings->char_block);
	volatile uint16_t *screen = screen_block_begin(settings->screen_block);
	screen += 32 * settings->from_top;

	// FIXME: What's the actual... size? *sweat drop*
	write_4bpp_n(settings->gfx->chars, out,
		     settings->gfx->height * BOX_GFX_WIDTH);

	assert(!settings->align_right && "unimplemented atm");
	assert(settings->width > BOX_GFX_WIDTH_M1);
	for (size_t x = 0; x < settings->width - BOX_GFX_WIDTH_M1; ++x) {
		for (size_t y = 0; y < settings->gfx->height; ++y) {
			screen[y * 32 + x] =
				settings->gfx->tiles[y * BOX_GFX_WIDTH]
				| PREP(TILE_PALETTE, settings->palette);
		}
	}
	for (size_t y = 0; y < settings->gfx->height; ++y) {
		for (size_t x = 1; x < BOX_GFX_WIDTH; ++x) {
			screen[y * 32 + x + (settings->width - BOX_GFX_WIDTH)] =
				settings->gfx->tiles[y * BOX_GFX_WIDTH + x]
				| PREP(TILE_PALETTE, settings->palette);
		}
	}
}