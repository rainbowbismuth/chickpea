#include "chickpea.h"
#include "chickpea/debug_font.h"

void write_debug_msg(const struct debug_font *font, uint32_t char_block,
		     uint32_t screen_block, uint32_t palette, uint32_t tile_x,
		     uint32_t tile_y, const char *msg)
{
	volatile struct character_4bpp *character =
		character_block_begin(char_block);
	volatile uint16_t *tile = screen_block_begin(screen_block);

	const struct character_4bpp empty = { 0 };
	write_4bpp(&empty, character);
	write_palette(&font->palette, bg_palette(palette));
	character++;
	for (size_t i = 0; *msg != '\0'; ++i, ++msg, ++tile_x) {
		if (tile_x >= 32) {
			tile_x = 0;
			tile_y += 1;
		}
		if (tile_y >= 32) {
			return;
		}

		write_4bpp(&font->characters[(size_t)*msg], &character[i]);
		uint32_t index = tile_y * 32 + tile_x;
		tile[index] = PREP(TILE_CHAR, i + 1) |
			      PREP(TILE_PALETTE, palette);
	}
}