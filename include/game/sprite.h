#ifndef CHICKPEA_SPRITE_H
#define CHICKPEA_SPRITE_H

#include "chickpea.h"
#include "chickpea/vec2.h"

#define MAX_SPRITES	   64
#define MAX_SPRITE_OBJECTS 8

struct sprite_object_def {
	int8_t x_offset;
	int8_t y_offset;
	enum obj_shape shape;
	enum obj_size size;
};

struct sprite_template {
	const struct sprite_object_def *nonnull objects;
	uint8_t num_objects;
	uint8_t palette;
	enum obj_mode mode;
	bool colors_256;
};

typedef struct sprite_handle_s {
	uint8_t generation;
	uint8_t index;
} sprite_handle;

struct sprite {
	struct vec2 pos;
	uint16_t order;
	uint8_t palette;
	enum obj_mode mode;
	bool enabled;
	bool flip_vertical;
	bool flip_horizontal;
	uint8_t priority[MAX_SPRITE_OBJECTS];
	uint16_t _padding[6];
};

size_t sprite_allocated(void);
bool sprite_exists(sprite_handle handle);
struct sprite *nonnull sprite_ref(sprite_handle handle);
sprite_handle sprite_alloc(const struct sprite_template *nonnull template);
struct char_4bpp *nonnull sprite_obj_vram(sprite_handle handle);
size_t sprite_num_tiles(sprite_handle handle);
void sprite_drop(sprite_handle handle);
void sprite_queue_frame_copy(sprite_handle handle,
			     const struct char_4bpp *nonnull src);
void sprite_execute_frame_copies(void);

/*
 * Resets all sprite data, including object_tiles.
 */
void sprite_reset(void);

void sprite_build_oam_buffer(void);
void sprite_commit_buffer_to_oam(void);

#endif // CHICKPEA_SPRITE_H
