#ifndef CHICKPEA_OBJECT_TILES_H
#define CHICKPEA_OBJECT_TILES_H

#include "chickpea.h"

#define MAX_OBJECTS 128

typedef struct obj_tiles_handle_s {
	uint8_t generation;
	uint8_t index;
} obj_tiles_handle;

bool obj_tiles_exists(obj_tiles_handle handle);
obj_tiles_handle obj_tiles_alloc(size_t num_tiles);
void obj_tiles_drop(obj_tiles_handle handle);
size_t obj_tiles_start(obj_tiles_handle handle);
size_t obj_tiles_count(obj_tiles_handle handle);
volatile struct char_4bpp *nonnull obj_tiles_vram(obj_tiles_handle handle);

void obj_tiles_reset(void);

#endif //CHICKPEA_OBJECT_TILES_H
