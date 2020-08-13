#include "game/text_box.h"

#define BOX_WIDTH    4
#define BOX_WIDTH_M1 (BOX_WIDTH - 1)
#define RIGHT_END    (GBA_WIDTH / 8)

static void text_box_draw_left(const struct text_box_config *config,
			       volatile uint16_t *screen)
{
	for (size_t y = 0; y < config->gfx->height; ++y) {
		for (size_t x = 0; x < config->width - BOX_WIDTH_M1; ++x) {
			screen[y * 32 + x] =
				config->gfx->tiles[y * BOX_WIDTH]
				| PREP(TILE_PALETTE, config->palette);
		}
	}
	for (size_t y = 0; y < config->gfx->height; ++y) {
		for (size_t x = 1; x < BOX_WIDTH; ++x) {
			screen[y * 32 + x + (config->width - BOX_WIDTH)] =
				config->gfx->tiles[y * BOX_WIDTH + x]
				| PREP(TILE_PALETTE, config->palette);
		}
	}
}

static void text_box_draw_right(const struct text_box_config *config,
				volatile uint16_t *screen)
{
	for (size_t y = 0; y < config->gfx->height; ++y) {
		for (size_t x = 0; x < config->width - BOX_WIDTH_M1; ++x) {
			screen[y * 32 + RIGHT_END - x] =
				config->gfx->tiles[y * BOX_WIDTH]
				| PREP(TILE_PALETTE, config->palette)
				| TILE_HORIZONTAL_FLIP;
		}
	}
	for (size_t y = 0; y < config->gfx->height; ++y) {
		for (size_t x = 1; x < BOX_WIDTH; ++x) {
			screen[y * 32 + RIGHT_END - x
			       - (config->width - BOX_WIDTH)] =
				config->gfx->tiles[y * BOX_WIDTH + x]
				| PREP(TILE_PALETTE, config->palette)
				| TILE_HORIZONTAL_FLIP;
		}
	}
}

void text_box_draw(const struct text_box_config *config)
{
	// FIXME: Hard coded to be first in block..?
	volatile struct char_4bpp *out = char_block_begin(config->char_block);
	volatile uint16_t *screen = screen_block_begin(config->screen_block);
	screen += 32 * config->from_top;
	
	assert(config->gfx->length != 0);
	write_4bpp_n(config->gfx->chars, out,
		     config->gfx->length / sizeof(*out));

	assert(config->width > BOX_WIDTH_M1);

	if (config->align_right) {
		text_box_draw_right(config, screen);
	} else {
		text_box_draw_left(config, screen);
	}
}