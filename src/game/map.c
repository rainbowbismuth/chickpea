#include "game/map.h"
#include "chickpea/bit_vec.h"
#include "chickpea/vec2.h"

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
	struct vec2 coords = { .x = 15 + (pos.x - pos.y) * 2,
			       .y = pos.x + pos.y };
	return coords;
}

struct vec2 to_screen_coord(struct map_byte_vec *nonnull height_map,
			    struct vec2 pos)
{
	assert(inside_map(pos));
	uint8_t height = height_map->bytes[pos.y][pos.x];
	struct vec2 tile_coord = to_tile_coord(pos);
	struct vec2 coords = { .x = tile_coord.x * 8,
			       .y = tile_coord.y * 8 - height * 8 };
	return coords;
}

void map_bit_vec_set(struct map_bit_vec *nonnull bitset, struct vec2 pos)
{
	assert(inside_map(pos));
	bit_vec_set(bitset->bits[pos.y], ARRAY_SIZE(bitset->bits[pos.y]),
		    pos.x);
}

void map_bit_vec_toggle(struct map_bit_vec *nonnull bitset, struct vec2 pos)
{
	assert(inside_map(pos));
	bit_vec_toggle(bitset->bits[pos.y], ARRAY_SIZE(bitset->bits[pos.y]),
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

extern struct char_4bpp tile_highlight_4bpp[2];
extern struct palette tile_highlight_pal;

extern uint16_t demo_map_low[32][32];
extern uint16_t demo_map_high[32][32];
extern struct char_4bpp demo_map_4bpp[];
extern struct palette demo_map_pal;

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

	// TODO: Should determine how many tiles there are ;)
	write_4bpp_n(demo_map_4bpp, character_block_begin(0), 128);
	write_palette(&demo_map_pal, bg_palette(0));
	cpu_fast_set(&demo_map_low, (void *)screen_block_begin(8),
		     sizeof(demo_map_low) / 4);
	cpu_fast_set(&demo_map_high, (void *)screen_block_begin(9),
		     sizeof(demo_map_high) / 4);
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

void demo_gross_fix_up_step(volatile uint16_t *screen_high, struct vec2 t_0_pos)
{
	if (demo_map_high[t_0_pos.y][t_0_pos.x] &&
	    demo_map_high[t_0_pos.y][t_0_pos.x + 1] &&
	    !demo_map_high[t_0_pos.y + 1][t_0_pos.x] &&
	    !demo_map_high[t_0_pos.y][t_0_pos.x + 2]) {
		screen_high[tile_to_screen(t_0_pos)] =
			PREP(TILE_CHAR, top_left[0]) | PREP(TILE_PALETTE, pal);
		screen_high[tile_to_screen(t_0_pos) + 1] =
			PREP(TILE_CHAR, top_left[1]) | PREP(TILE_PALETTE, pal);
	} else if (demo_map_high[t_0_pos.y][t_0_pos.x + 2] &&
		   demo_map_high[t_0_pos.y][t_0_pos.x + 3] &&
		   !demo_map_high[t_0_pos.y + 1][t_0_pos.x + 2] &&
		   !demo_map_high[t_0_pos.y][t_0_pos.x + 1]) {
		screen_high[tile_to_screen(t_0_pos) + 2] =
			PREP(TILE_CHAR, top_left[1]) | PREP(TILE_PALETTE, pal) |
			TILE_HORIZONTAL_FLIP;
		screen_high[tile_to_screen(t_0_pos) + 3] =
			PREP(TILE_CHAR, top_left[0]) | PREP(TILE_PALETTE, pal) |
			TILE_HORIZONTAL_FLIP;
	}
}

void demo_render_tile_highlights(struct map_render_params *nonnull params,
				 struct map_bit_vec *nonnull highlights,
				 struct map_byte_vec *nonnull height_map,
				 struct map_bit_vec *nonnull occlusion)
{
	volatile uint16_t *screen_low =
		screen_block_begin(params->screen_block_low);
	volatile uint16_t *screen_high =
		screen_block_begin(params->screen_block_high);
	cpu_fast_fill(0, (void *)screen_low, (16 * 32 * 32) / 4);
	cpu_fast_fill(0, (void *)screen_high, (16 * 32 * 32) / 4);

	volatile struct char_4bpp *chars =
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
			struct vec2 t_0_pos = to_tile_coord(pos);
			t_0_pos.y -= height_map->bytes[y][x];

			if (demo_map_high[t_0_pos.y][t_0_pos.x + 2] &&
			    demo_map_high[t_0_pos.y][t_0_pos.x + 3] &&
			    demo_map_high[t_0_pos.y + 1][t_0_pos.x] &&
			    demo_map_high[t_0_pos.y + 1][t_0_pos.x + 1] &&
			    demo_map_high[t_0_pos.y + 1][t_0_pos.x + 2] &&
			    demo_map_high[t_0_pos.y + 1][t_0_pos.x + 3]) {
				demo_render_one_highlight(screen_high, t_0_pos);
			} else {
				demo_render_one_highlight(screen_low, t_0_pos);
				demo_gross_fix_up_step(screen_high, t_0_pos);
			}
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

extern struct char_4bpp tile_cursor_4bpp[8];
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
	sprite_queue_frame_copy(h, &tile_cursor_4bpp[0]);
	write_palette(&tile_cursor_pal, obj_palette(1));
	return h;
}

void demo_move_cursor(struct map_byte_vec *nonnull height_map,
		      struct map_bit_vec *nonnull occlusion,
		      sprite_handle cursor, struct vec2 pos, struct vec2 scroll)
{
	struct vec2 screen_coords = to_screen_coord(height_map, pos);
	screen_coords.x -= scroll.x;
	screen_coords.y -= scroll.y;
	struct sprite *sprite = sprite_ref(cursor);
	sprite->pos = screen_coords;
}

const static struct sprite_object_def soldier_objs[4] = {
	{ .x_offset = 0,
	  .y_offset = 0,
	  .shape = OBJ_SHAPE_VERTICAL,
	  .size = OBJ_SIZE_8 },
	{ .x_offset = 8,
	  .y_offset = 0,
	  .shape = OBJ_SHAPE_VERTICAL,
	  .size = OBJ_SIZE_8 },
	{ .x_offset = 0,
	  .y_offset = 16,
	  .shape = OBJ_SHAPE_VERTICAL,
	  .size = OBJ_SIZE_8 },
	{ .x_offset = 8,
	  .y_offset = 16,
	  .shape = OBJ_SHAPE_VERTICAL,
	  .size = OBJ_SIZE_8 },
};

const static struct sprite_template soldier_template = {
	.objects = soldier_objs,
	.num_objects = ARRAY_SIZE(soldier_objs),
	.palette = 2,
	.mode = OBJ_MODE_NORMAL,
};

extern struct char_4bpp soldier_4bpp[8 * 6];
extern struct palette soldier_pal;

sprite_handle demo_alloc_soldier(void)
{
	return sprite_alloc(&soldier_template);
}

void demo_move_soldier(struct map_byte_vec *nonnull height_map,
		       struct map_bit_vec *nonnull occlusion,
		       sprite_handle soldier, struct vec2 pos,
		       struct vec2 scroll)
{
	struct vec2 screen_coords = to_screen_coord(height_map, pos);
	screen_coords.x -= scroll.x;
	screen_coords.y -= scroll.y;
	screen_coords.x += 8;
	screen_coords.y -= (32 - 11);
	struct sprite *sprite = sprite_ref(soldier);
	sprite->pos = screen_coords;
	for (size_t i = 0; i < 4; ++i) {
		sprite->priority[i] = 2;
	}
	if (map_bit_vec_test(occlusion, pos)) {
		sprite->priority[2] = 3;
		sprite->priority[3] = 3;
	}
}

void demo_soldier_frame(sprite_handle soldier, enum facing facing,
			uint32_t frame)
{
	struct sprite *sprite = sprite_ref(soldier);
	sprite->flip_horizontal = facing == FACING_NORTH ||
				  facing == FACING_EAST;
	uint32_t offset =
		facing == FACING_SOUTH || facing == FACING_EAST ? 0 : 3 * 8;
	offset += frame * 8;

	sprite_queue_frame_copy(soldier, &soldier_4bpp[offset]);
	write_palette(&soldier_pal, obj_palette(sprite->palette));
}