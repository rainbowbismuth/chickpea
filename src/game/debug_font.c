#include "chickpea.h"
#include "game/debug_font.h"

void write_debug_msg(const struct debug_font *nonnull font, uint32_t char_block,
		     uint32_t screen_block, uint32_t palette, uint32_t tile_x,
		     uint32_t tile_y, const char *nonnull msg)
{
	volatile struct char_4bpp *character =
		character_block_begin(char_block);
	volatile uint16_t *tile = screen_block_begin(screen_block);

	const struct char_4bpp empty = { 0 };
	write_4bpp(&empty, character);
	write_palette(font->palette, bg_palette(palette));
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

sprite_handle
write_debug_msg_sprite(const struct debug_font *nonnull font,
		       const struct sprite_template *nonnull template,
		       const char *nonnull msg)
{
	sprite_handle h = sprite_alloc(template);
	volatile struct char_4bpp *vram = sprite_obj_vram(h);
	write_palette(font->palette, obj_palette(template->palette));
	size_t num_tiles = sprite_num_tiles(h);
	size_t count = 0;
	for (; *msg != '\0'; ++msg, ++vram, ++count) {
		write_4bpp(&font->characters[(size_t)*msg], vram);
	}
	assert(count <= num_tiles);
	if (count < num_tiles) {
		cpu_fast_fill(0, (void *)vram,
			      (num_tiles - count) * sizeof(*vram) / 4);
	}
	return h;
}