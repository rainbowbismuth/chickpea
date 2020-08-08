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

static struct vec2
to_screen_coord(const struct map_byte_vec *nonnull height_map, struct vec2 pos)
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

extern struct map_tiles demo_map_low;
extern struct map_tiles demo_map_high;
extern struct map_byte_vec demo_map_height;
extern struct map_byte_vec demo_map_attributes;
extern struct char_4bpp demo_map_4bpp[];
extern struct palette demo_map_pal;

struct map demo_map = {
	.lower = &demo_map_low,
	.upper = &demo_map_high,
	.height = &demo_map_height,
	.attributes = &demo_map_attributes,
};

void demo_init(void)
{
	tile_highlight_gfx.palette = tile_highlight_pal;
	tile_highlight_gfx.top_left[0] = tile_highlight_4bpp[0];
	tile_highlight_gfx.top_left[1] = tile_highlight_4bpp[1];

	tile_highlight_gfx.top_left_together[0] = tile_highlight_4bpp[1];
	ch4bpp_flip_both(&tile_highlight_gfx.top_left_together[0]);
	ch4bpp_bitor(&tile_highlight_gfx.top_left_together[0],
		     &tile_highlight_4bpp[0]);

	tile_highlight_gfx.top_left_together[1] = tile_highlight_4bpp[0];
	ch4bpp_flip_both(&tile_highlight_gfx.top_left_together[1]);
	ch4bpp_bitor(&tile_highlight_gfx.top_left_together[1],
		     &tile_highlight_4bpp[1]);

	// TODO: Should determine how many tiles there are ;)
	write_4bpp_n(demo_map_4bpp, char_block_begin(0), 128);
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

void demo_gross_fix_up_step(struct map *nonnull map,
			    volatile uint16_t *screen_high, struct vec2 t_0_pos)
{
	if (map->upper->tiles[t_0_pos.y][t_0_pos.x] &&
	    map->upper->tiles[t_0_pos.y][t_0_pos.x + 1] &&
	    !map->upper->tiles[t_0_pos.y + 1][t_0_pos.x] &&
	    !map->upper->tiles[t_0_pos.y][t_0_pos.x + 2]) {
		screen_high[tile_to_screen(t_0_pos)] =
			PREP(TILE_CHAR, top_left[0]) | PREP(TILE_PALETTE, pal);
		screen_high[tile_to_screen(t_0_pos) + 1] =
			PREP(TILE_CHAR, top_left[1]) | PREP(TILE_PALETTE, pal);
	} else if (map->upper->tiles[t_0_pos.y][t_0_pos.x + 2] &&
		   map->upper->tiles[t_0_pos.y][t_0_pos.x + 3] &&
		   !map->upper->tiles[t_0_pos.y + 1][t_0_pos.x + 2] &&
		   !map->upper->tiles[t_0_pos.y][t_0_pos.x + 1]) {
		screen_high[tile_to_screen(t_0_pos) + 2] =
			PREP(TILE_CHAR, top_left[1]) | PREP(TILE_PALETTE, pal) |
			TILE_HORIZONTAL_FLIP;
		screen_high[tile_to_screen(t_0_pos) + 3] =
			PREP(TILE_CHAR, top_left[0]) | PREP(TILE_PALETTE, pal) |
			TILE_HORIZONTAL_FLIP;
	}
}

void demo_render_tile_highlights(struct map *nonnull map,
				 struct map_render_params *nonnull params,
				 struct map_bit_vec *nonnull highlights)
{
	volatile uint16_t *screen_low = screen_block_begin(params->screen_low);
	volatile uint16_t *screen_high =
		screen_block_begin(params->screen_high);
	cpu_fast_fill(0, (void *)screen_low, (16 * 32 * 32) / 4);
	cpu_fast_fill(0, (void *)screen_high, (16 * 32 * 32) / 4);

	volatile struct char_4bpp *chars = char_block_begin(params->char_block);

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
			t_0_pos.y -= map->height->bytes[y][x];

			if (map->upper->tiles[t_0_pos.y][t_0_pos.x + 2] &&
			    map->upper->tiles[t_0_pos.y][t_0_pos.x + 3] &&
			    map->upper->tiles[t_0_pos.y + 1][t_0_pos.x] &&
			    map->upper->tiles[t_0_pos.y + 1][t_0_pos.x + 1] &&
			    map->upper->tiles[t_0_pos.y + 1][t_0_pos.x + 2] &&
			    map->upper->tiles[t_0_pos.y + 1][t_0_pos.x + 3]) {
				demo_render_one_highlight(screen_high, t_0_pos);
			} else {
				demo_render_one_highlight(screen_low, t_0_pos);
				demo_gross_fix_up_step(map, screen_high,
						       t_0_pos);
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
	{ .x_offset = 0,
	  .y_offset = 0,
	  .shape = OBJ_SHAPE_HORIZONTAL,
	  .size = OBJ_SIZE_8 },
	{ .x_offset = 16,
	  .y_offset = 0,
	  .shape = OBJ_SHAPE_HORIZONTAL,
	  .size = OBJ_SIZE_8 },
	{ .x_offset = 0,
	  .y_offset = 8,
	  .shape = OBJ_SHAPE_HORIZONTAL,
	  .size = OBJ_SIZE_8 },
	{ .x_offset = 16,
	  .y_offset = 8,
	  .shape = OBJ_SHAPE_HORIZONTAL,
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

extern struct char_4bpp tile_pointer_4bpp[2];

const static struct sprite_object_def pointer_objs[1] = {
	{ .x_offset = 0,
	  .y_offset = 0,
	  .shape = OBJ_SHAPE_VERTICAL,
	  .size = OBJ_SIZE_8 }
};

const static struct sprite_template pointer_template = {
	.objects = pointer_objs,
	.num_objects = ARRAY_SIZE(pointer_objs),
	.palette = 1,
	.mode = OBJ_MODE_NORMAL,
};

sprite_handle demo_alloc_pointer(void)
{
	sprite_handle h = sprite_alloc(&pointer_template);
	sprite_queue_frame_copy(h, &tile_pointer_4bpp[0]);
	return h;
}

static int8_t pointer_bounce_anim[22] = { -5, -5, -5, -5, -5, -5, -4, -4,
					  -4, -4, -3, -3, -2, -2, -1, 0,
					  0,  0,  0,  -1, -4, -4 };

void demo_move_pointer(struct map *nonnull map, sprite_handle pointer,
		       struct vec2 pos, struct vec2 scroll, uint32_t frame)
{
	struct vec2 screen_coords = to_screen_coord(map->height, pos);
	screen_coords.x -= scroll.x;
	screen_coords.y -= scroll.y;
	screen_coords.x += 12;
	screen_coords.y -= 32;
	screen_coords.y += pointer_bounce_anim[(frame >> 1) %
					       ARRAY_SIZE(pointer_bounce_anim)];

	struct sprite *sprite = sprite_ref(pointer);
	sprite->pos = screen_coords;
	sprite->order = 0;
}

void demo_move_cursor(struct map *nonnull map, sprite_handle cursor,
		      struct vec2 pos, struct vec2 scroll)
{
	struct vec2 screen_coords = to_screen_coord(map->height, pos);
	screen_coords.x -= scroll.x;
	screen_coords.y -= scroll.y;
	struct sprite *sprite = sprite_ref(cursor);
	sprite->pos = screen_coords;
	sprite->order = ((32 - pos.y) << 8) + ((32 - pos.x) << 2) + 1;
	for (size_t i = 0; i < 4; ++i) {
		sprite->priority[i] = 3;
	}
	struct vec2 t_0_pos = to_tile_coord(pos);
	t_0_pos.y -= map->height->bytes[pos.y][pos.x];
	uint8_t attr = map->attributes->bytes[pos.y][pos.x];
	if (map->upper->tiles[t_0_pos.y][t_0_pos.x + 1] &&
	    ~attr & MAP_ATTR_OCCLUDED_BOT_LEFT) {
		sprite->priority[0] = 2;
	}
	if (map->upper->tiles[t_0_pos.y][t_0_pos.x + 2] &&
	    ~attr & MAP_ATTR_OCCLUDED_BOT_RIGHT) {
		sprite->priority[1] = 2;
	}
	if (map->upper->tiles[t_0_pos.y + 1][t_0_pos.x + 1] &&
	    ~attr & MAP_ATTR_OCCLUDED_BOT_LEFT) {
		sprite->priority[2] = 2;
	}
	if (map->upper->tiles[t_0_pos.y + 1][t_0_pos.x + 2] &&
	    ~attr & MAP_ATTR_OCCLUDED_BOT_RIGHT) {
		sprite->priority[3] = 2;
	}
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

void demo_move_soldier(struct map *nonnull map, sprite_handle soldier,
		       struct vec2 pos, struct vec2 scroll)
{
	struct vec2 screen_coords = to_screen_coord(map->height, pos);
	screen_coords.x -= scroll.x;
	screen_coords.y -= scroll.y;
	screen_coords.x += 8;
	screen_coords.y -= (32 - 11);
	struct sprite *sprite = sprite_ref(soldier);
	sprite->pos = screen_coords;
	sprite->order = ((32 - pos.y) << 8) + ((32 - pos.x) << 2);
	for (size_t i = 0; i < 4; ++i) {
		sprite->priority[i] = 2;
	}
	uint8_t attr = map->attributes->bytes[pos.y][pos.x];
	size_t bottom_left = 2;
	size_t bottom_right = 3;
	if (sprite->flip_horizontal) {
		bottom_left = 3;
		bottom_right = 2;
	}
	if (attr & MAP_ATTR_OCCLUDED_BOT_LEFT) {
		sprite->priority[bottom_left] = 3;
	}
	if (attr & MAP_ATTR_OCCLUDED_BOT_RIGHT) {
		sprite->priority[bottom_right] = 3;
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