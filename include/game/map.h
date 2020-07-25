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

struct tile_highlight_gfx {
	struct palette palette;
	struct char_4bpp top_left[2];
	struct char_4bpp top_left_together[2];
};

bool inside_map(struct vec2 pos);

struct vec2 to_tile_coord(struct vec2 pos);

void map_bit_vec_set(struct map_bit_vec *nonnull bitset, struct vec2 pos);

bool map_bit_vec_test(struct map_bit_vec *nonnull bitset, struct vec2 pos);

struct map_render_params {
	uint32_t screen_block;
	uint32_t char_block;
};

/**
 * A temporary function just to try rendering this out
 * @param highlights The currently highlighted tiles.
 */
void demo_init(void);

void demo_render_tile_highlights(struct map_render_params *nonnull params,
				 struct map_bit_vec *nonnull highlights,
				 struct map_byte_vec *nonnull height_map);
void demo_rotate_highlight_palette(uint32_t offset);

sprite_handle demo_alloc_cursor(void);

#endif //CHICKPEA_MAP_H
