#include "game/resource.h"

/*
 * TODO: Not sure how I'm going to need to structure this,
 * 	so the 'handle' idea might be more than necessary.
 * 	Particularly given the 'free everything as once' API.
 */

#define ARENA_SIZE (64 * 1024)
static uint8_t arena[ARENA_SIZE] EWRAM = { 0 };
static uint8_t *top = &arena[0];

struct resource_alloc {
	const void *nullable data;
	struct resource *nullable src;
	uint16_t generation;
	bool used;
};

#define MAX_ALLOCS 512
static struct resource_alloc allocs[MAX_ALLOCS] = { 0 };

static void *arena_alloc(uint32_t bytes)
{
	bytes = (bytes + 3) & ~0x3;
	void *alloc = top;
	top += bytes;
	assert(top < &arena[ARENA_SIZE - 1]);
	return alloc;
}

static bool alloc_exists(resource_handle handle)
{
	assert(handle.index < MAX_ALLOCS);
	return allocs[handle.index].generation == handle.generation
	    && allocs[handle.index].used;
}

static size_t find_first_unused(void)
{
	for (size_t i = 0; i < MAX_ALLOCS; ++i) {
		if (!allocs[i].used) {
			return i;
		}
	}
	assert(false && "no unused resource allocs");
}

const void *resource_data(struct resource *nonnull resource)
{
	if (!resource->lz77) {
		return resource->data;
	}
	if (alloc_exists(resource->allocation)) {
		return allocs[resource->allocation.index].data;
	}

	void *dst = arena_alloc(resource->length);
	decompress_lz77_wram(resource->data, dst);

	size_t i = find_first_unused();
	allocs[i].used = true;
	allocs[i].src = resource;
	allocs[i].data = dst;

	// Skip over zero on allocation so that { 0 } is never a valid handle
	if (allocs[i].generation == 0) {
		allocs[i].generation++;
	}
	
	resource->allocation =
		(resource_handle){ .index = i,
				   .generation = allocs[i].generation };
	return dst;
}

void resource_copy_to_vram(const struct resource *nonnull resource,
			   void *nonnull vram)
{
	if (!resource->lz77) {
		// TODO: making some mighty assumptions here.
		cpu_fast_set(resource->data, vram, resource->length / 4);
		return;
	}
	decompress_lz77_vram(resource->data, vram);
}

void resource_reset(void)
{
	top = &arena[0];
	for (size_t i = 0; i < MAX_ALLOCS; ++i) {
		if (!allocs[i].used) {
			continue;
		}
		allocs[i].src->allocation = (resource_handle){ 0 };
		allocs[i].src = NULL;
		allocs[i].generation++;
		allocs[i].used = false;
	}
}