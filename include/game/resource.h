#ifndef CHICKPEA_RESOURCE_H
#define CHICKPEA_RESOURCE_H

#include "chickpea.h"

typedef struct resource_handle_s {
	uint16_t generation;
	uint16_t index;
} resource_handle;

struct resource {
	const uint32_t length;
	const bool lz77;
	const void *const nonnull data;
	resource_handle allocation;
};

const void *nonnull resource_data(struct resource *nonnull resource);
void resource_copy_to_vram(const struct resource *nonnull resource,
			   void *nonnull vram);
void resource_reset(void);

#endif // CHICKPEA_RESOURCE_H
