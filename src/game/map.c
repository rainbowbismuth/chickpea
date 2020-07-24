#include "game/map.h"
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

static struct tile_highlight_gfx tile_highlight_gfx = { 0 };
static const size_t pal = 9;
static const size_t top_left[2] = { 1, 2 };
static const size_t top_left_together[2] = { 3, 4 };

extern struct character_4bpp tile_highlight_4bpp[2];
extern struct palette tile_highlight_pal;

void demo_init(void)
{
	tile_highlight_gfx.palette = tile_highlight_pal;
	tile_highlight_gfx.top_left[0] = tile_highlight_4bpp[0];
	tile_highlight_gfx.top_left[1] = tile_highlight_4bpp[1];

	tile_highlight_gfx.top_left_together[0] = tile_highlight_4bpp[1];
	char_4bpp_flip_both(&tile_highlight_gfx.top_left_together[0]);
	char_4bpp_bitwise_or(&tile_highlight_gfx.top_left_together[0],
			     &tile_highlight_4bpp[0]);

	tile_highlight_gfx.top_left_together[1] = tile_highlight_4bpp[0];
	char_4bpp_flip_both(&tile_highlight_gfx.top_left_together[1]);
	char_4bpp_bitwise_or(&tile_highlight_gfx.top_left_together[1],
			     &tile_highlight_4bpp[1]);
}

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

extern struct character_4bpp tile_cursor_4bpp[8];
extern struct palette tile_cursor_pal;

const static struct sprite_object_def cursor_objs[4] = {
	{ .x_offset = 8,
	  .y_offset = -2,
	  .shape = OBJ_SHAPE_HORIZONTAL,
	  .size = OBJ_SIZE_8 },
	{ .x_offset = 8,
	  .y_offset = 10,
	  .shape = OBJ_SHAPE_HORIZONTAL,
	  .size = OBJ_SIZE_8 },
	{ .x_offset = -2,
	  .y_offset = 0,
	  .shape = OBJ_SHAPE_VERTICAL,
	  .size = OBJ_SIZE_8 },
	{ .x_offset = 26,
	  .y_offset = 0,
	  .shape = OBJ_SHAPE_VERTICAL,
	  .size = OBJ_SIZE_8 },
};

const static struct sprite_template cursor_template = {
	.objects = cursor_objs,
	.num_objects = ARRAY_SIZE(cursor_objs),
	.palette = 1,
	.mode = OBJ_MODE_NORMAL,
};

sprite_handle demo_alloc_cursor(void)
{
	sprite_handle h = sprite_alloc(&cursor_template);
	write_4bpp_n(&tile_cursor_4bpp[1], sprite_obj_vram(h, 0), 2);
	write_4bpp_n(&tile_cursor_4bpp[5], sprite_obj_vram(h, 1), 2);
	write_4bpp(&tile_cursor_4bpp[0], sprite_obj_vram(h, 2) + 0);
	write_4bpp(&tile_cursor_4bpp[4], sprite_obj_vram(h, 2) + 1);
	write_4bpp(&tile_cursor_4bpp[3], sprite_obj_vram(h, 3) + 0);
	write_4bpp(&tile_cursor_4bpp[7], sprite_obj_vram(h, 3) + 1);
	write_palette(&tile_cursor_pal, obj_palette(1));
	return h;
}