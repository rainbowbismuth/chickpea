#include "lua/alloc.h"
#include "chickpea.h"
#include <string.h>

#define ALIGNMENT  8
#define ARENA_SIZE (64 * 1024)
static uint8_t arena[ARENA_SIZE] EWRAM
	__attribute__((aligned(ALIGNMENT))) = { 0 };
static uint8_t *top = &arena[ARENA_SIZE];

static void *arena_alloc(uint32_t bytes)
{
	top -= bytes;
	top = (uint8_t *)(((uintptr_t)top) & ~(ALIGNMENT - 1));
	assert(top >= &arena[0]);
	return top;
}

static uint32_t *get_size(void *ptr)
{
	return ((uint32_t *)ptr) - 2;
}

static void *alloc_size(uint32_t size)
{
	void *alloc = arena_alloc(size + sizeof(uint64_t)) + sizeof(uint64_t);
	*get_size(alloc) = size;
	return alloc;
}

void lua_alloc_debug_mem_use(void)
{
	size_t size = ((uintptr_t)&arena[ARENA_SIZE]) - ((uintptr_t)top);
	debug_put_str("lua_alloc_debug_mem_use: ");
	debug_put_u32(size);
	debug_put_str(" bytes\n");
}

void lua_alloc_free(void *ptr)
{
	if (!ptr) {
		return;
	}
	uint32_t size = *get_size(ptr);
	debug_put_str("lua_alloc_free: ");
	debug_put_u32(size);
	debug_put_str(" bytes\n");
	return;
}

void *lua_alloc_realloc(void *ptr, size_t size)
{
	debug_put_str("lua_alloc_realloc: ");
	debug_put_u32(size);
	debug_put_str(" bytes ");
	if (ptr) {
		uint32_t old_size = *get_size(ptr);
		if (old_size > size) {
			// Don't shrink
			debug_put_str("(shrink)\n");
			return ptr;
		}
		void *new = alloc_size(size);
		memcpy(new, ptr, old_size);
		debug_put_str("(grow)\n");
		return new;
	}
	debug_put_str("(new)\n");
	return alloc_size(size);
}
