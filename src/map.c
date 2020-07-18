#include "chickpea/map.h"
#include "chickpea/bit_vec.h"

bool inside_map(struct vec2 pos)
{
	if (pos.x < 0 || pos.y < 0) {
		return false;
	} else if (pos.x > MAP_WIDTH || pos.y > MAP_HEIGHT) {
		return false;
	} else {
		return true;
	}
}

struct vec2 to_tile_coord(struct vec2 pos)
{
	struct vec2 coords = { .x = 16 + (pos.x - pos.y) * 2,
			       .y = pos.x + pos.y };
	return coords;
}

void map_bit_vec_set(struct map_bit_vec *nonnull bitset, struct vec2 pos)
{
	assert(inside_map(pos));
	bit_vec_set(bitset->bits[pos.y], ARRAY_SIZE(bitset->bits[pos.y]),
		    pos.x);
}

bool map_bit_vec_test(struct map_bit_vec *nonnull bitset, struct vec2 pos)
{
	assert(inside_map(pos));
	return bit_vec_test(bitset->bits[pos.y],
			    ARRAY_SIZE(bitset->bits[pos.y]), pos.x);
}

extern struct tile_highlight_gfx tile_highlight_gfx;
static const size_t pal = 9;
static const size_t top_left[2] = { 1, 2 };
static const size_t top_left_together[2] = { 3, 4 };

size_t tile_to_screen(struct vec2 pos)
{
	return (pos.x % MAP_WIDTH) + (pos.y % MAP_HEIGHT) * MAP_HEIGHT;
}

void demo_render_one_highlight(volatile uint16_t *screen,
			       struct vec2 tile_0_pos)
{
	size_t tile_0_idx = tile_to_screen(tile_0_pos);
	size_t tile_1_idx = tile_to_screen(v2_add_x(tile_0_pos, 1));
	uint16_t tile_0 = screen[tile_0_idx];
	uint16_t tile_1 = screen[tile_1_idx];
	if (GET(TILE_CHAR, tile_0) == top_left[1]) {
		screen[tile_0_idx] =
			REPLACE(TILE_CHAR, tile_0, top_left_together[1]);
		screen[tile_1_idx] =
			REPLACE(TILE_CHAR, tile_1, top_left_together[0]);
	} else {
		screen[tile_0_idx] = PREP(TILE_CHAR, top_left[0]) |
				     PREP(TILE_PALETTE, pal);
		screen[tile_1_idx] = PREP(TILE_CHAR, top_left[1]) |
				     PREP(TILE_PALETTE, pal);
	}

	size_t tile_2_idx = tile_to_screen(v2_add_x(tile_0_pos, 2));
	size_t tile_3_idx = tile_to_screen(v2_add_x(tile_0_pos, 3));
	uint16_t tile_2 = screen[tile_2_idx];
	uint16_t tile_3 = screen[tile_3_idx];
	if (GET(TILE_CHAR, tile_2) == top_left[0]) {
		screen[tile_2_idx] =
			REPLACE(TILE_CHAR, tile_2, top_left_together[0]);
		screen[tile_3_idx] =
			REPLACE(TILE_CHAR, tile_3, top_left_together[1]);
	} else {
		screen[tile_2_idx] = PREP(TILE_CHAR, top_left[1]) |
				     PREP(TILE_PALETTE, pal) |
				     TILE_HORIZONTAL_FLIP;
		screen[tile_3_idx] = PREP(TILE_CHAR, top_left[0]) |
				     PREP(TILE_PALETTE, pal) |
				     TILE_HORIZONTAL_FLIP;
	}

	screen[tile_to_screen(v2_add_xy(tile_0_pos, 0, 1))] =
		PREP(TILE_CHAR, top_left[0]) | PREP(TILE_PALETTE, pal) |
		TILE_VERTICAL_FLIP;
	screen[tile_to_screen(v2_add_xy(tile_0_pos, 1, 1))] =
		PREP(TILE_CHAR, top_left[1]) | PREP(TILE_PALETTE, pal) |
		TILE_VERTICAL_FLIP;
	screen[tile_to_screen(v2_add_xy(tile_0_pos, 2, 1))] =
		PREP(TILE_CHAR, top_left[1]) | PREP(TILE_PALETTE, pal) |
		TILE_VERTICAL_FLIP | TILE_HORIZONTAL_FLIP;
	screen[tile_to_screen(v2_add_xy(tile_0_pos, 3, 1))] =
		PREP(TILE_CHAR, top_left[0]) | PREP(TILE_PALETTE, pal) |
		TILE_VERTICAL_FLIP | TILE_HORIZONTAL_FLIP;
}

void demo_render_tile_highlights(struct map_render_params *nonnull params,
				 struct map_bit_vec *nonnull highlights,
				 struct map_byte_vec *nonnull height_map)
{
	for (size_t y = 4; y < 8; ++y) {
		for (size_t x = 4; x < 8; ++x) {
			struct vec2 pos = { .x = x, .y = y };
			map_bit_vec_set(highlights, pos);
		}
	}
	for (size_t y = 7; y >= 4; --y) {
		height_map->bytes[y][4] = 7 - y;
		height_map->bytes[y][5] = 7 - y;
	}

	volatile uint16_t *screen = screen_block_begin(params->screen_block);
	volatile struct character_4bpp *chars =
		character_block_begin(params->char_block);

	/*
	 * Ideally this wouldn't go here of course
	 */
	write_palette(&tile_highlight_gfx.palette, bg_palette(pal));
	write_4bpp(&tile_highlight_gfx.top_left[0], &chars[top_left[0]]);
	write_4bpp(&tile_highlight_gfx.top_left[1], &chars[top_left[1]]);
	write_4bpp(&tile_highlight_gfx.top_left_together[0],
		   &chars[top_left_together[0]]);
	write_4bpp(&tile_highlight_gfx.top_left_together[1],
		   &chars[top_left_together[1]]);

	for (size_t y = 0; y < MAP_HEIGHT; ++y) {
		for (size_t x = 0; x < MAP_WIDTH; ++x) {
			struct vec2 pos = { .x = x, .y = y };
			if (!map_bit_vec_test(highlights, pos)) {
				continue;
			}
			struct vec2 tile_0_pos = to_tile_coord(pos);
			tile_0_pos.y -= height_map->bytes[y][x];
			demo_render_one_highlight(screen, tile_0_pos);
		}
	}
}

void demo_rotate_highlight_palette(uint32_t offset)
{
	struct palette palette = {};
	offset /= 9;
	for (size_t i = 0; i < 8; ++i) {
		palette.color[8 - i] =
			tile_highlight_gfx.palette.color[1 + (i + offset) % 7];
	}
	write_palette(&palette, bg_palette(pal));
}