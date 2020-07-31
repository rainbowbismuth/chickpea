#ifndef CHICKPEA_MAP_H
#define CHICKPEA_MAP_H

#include "chickpea.h"
#include "chickpea/vec2.h"
#include "game/sprite.h"

#define MAP_WIDTH  32
#define MAP_HEIGHT 32

struct map_bit_vec {
	uint8_t bits[MAP_HEIGHT][MAP_WIDTH / 8];
};

struct map_byte_vec {
	uint8_t bytes[MAP_HEIGHT][MAP_WIDTH];
};

// These are ideally not the same as MAP_WIDTH/MAP_HEIGHT
#define MAP_TILES_WIDTH	 32
#define MAP_TILES_HEIGHT 32

struct map_tiles {
	uint16_t tiles[MAP_TILES_WIDTH][MAP_TILES_HEIGHT];
};

struct map {
	const struct map_tiles *nonnull lower;
	const struct map_tiles *nonnull upper;
	const struct map_byte_vec *nonnull height;
	const struct map_byte_vec *nonnull attributes;
};

#define MAP_ATTR_WALK		    BIT(0)
#define MAP_ATTR_OCCLUDED_BOT_LEFT  BIT(1)
#define MAP_ATTR_OCCLUDED_BOT_RIGHT BIT(2)

struct tile_highlight_gfx {
	struct palette palette;
	struct char_4bpp top_left[2];
	struct char_4bpp top_left_together[2];
};

bool inside_map(struct vec2 pos);

struct vec2 to_tile_coord(struct vec2 pos);

void map_bit_vec_set(struct map_bit_vec *nonnull bitset, struct vec2 pos);

void map_bit_vec_toggle(struct map_bit_vec *nonnull bitset, struct vec2 pos);

bool map_bit_vec_test(struct map_bit_vec *nonnull bitset, struct vec2 pos);

struct map_render_params {
	uint32_t screen_block_low;
	uint32_t screen_block_high;
	uint32_t char_block;
};

enum facing { FACING_NORTH = 0, FACING_EAST, FACING_SOUTH, FACING_WEST };

/**
 * A temporary function just to try rendering this out
 * @param highlights The currently highlighted tiles.
 */
struct map demo_map;

void demo_init(void);

void demo_render_tile_highlights(struct map *nonnull map,
				 struct map_render_params *nonnull params,
				 struct map_bit_vec *nonnull highlights);
void demo_rotate_highlight_palette(uint32_t offset);

sprite_handle demo_alloc_cursor(void);
sprite_handle demo_alloc_pointer(void);

void demo_move_cursor(struct map *nonnull map, sprite_handle cursor,
		      struct vec2 pos, struct vec2 scroll);
void demo_move_pointer(struct map *nonnull map, sprite_handle pointer,
		       struct vec2 pos, struct vec2 scroll,uint32_t frame);

sprite_handle demo_alloc_soldier(void);
void demo_move_soldier(struct map *nonnull map, sprite_handle soldier,
		       struct vec2 pos, struct vec2 scroll);
void demo_soldier_frame(sprite_handle soldier, enum facing facing,
			uint32_t frame);

#endif //CHICKPEA_MAP_H
