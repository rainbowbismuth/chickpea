#include "lua/alloc.h"
#include "chickpea.h"
#include "tlsf.h"

#define ALIGNMENT  8
#define ARENA_SIZE (32 * 1024)
static uint8_t arena[ARENA_SIZE] EWRAM
	__attribute__((aligned(ALIGNMENT))) = { 0 };

static tlsf_t allocator;

void lua_alloc_init(void)
{
	allocator = tlsf_create_with_pool(arena, sizeof(arena));
}

void lua_alloc_free(void *ptr)
{
	debug_put_str("lua_alloc_free\n");
	tlsf_free(allocator, ptr);
}

void *lua_alloc_realloc(void *ptr, size_t new_size)
{
	return tlsf_realloc(allocator, ptr, new_size);
}
