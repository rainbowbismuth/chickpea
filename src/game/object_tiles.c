#include "game/object_tiles.h"
#include "chickpea.h"
#include "chickpea/bit_vec.h"
#include "chickpea/nano_unit.h"

#define OBJ_TILES 1024
static uint8_t used_tiles[OBJ_TILES / 8] = { 0 };

#define OBJ_TILES_COUNT	  FIELD(0, 7)
#define OBJ_TILES_ENABLED BIT(7)

struct object_tiles_alloc {
	uint16_t start;
	uint8_t attr;
	uint8_t generation;
};

static size_t allocated = 0;
static struct object_tiles_alloc allocs[MAX_OBJECTS] = { 0 };

bool obj_tiles_exists(obj_tiles_handle handle)
{
	// TODO: Remove the obj_tiles_enabled part.
	return allocs[handle.index].generation == handle.generation
	    && GET(OBJ_TILES_ENABLED, allocs[handle.index].attr);
}

static size_t find_disabled_alloc_idx(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(allocs); ++i) {
		if (!GET(OBJ_TILES_ENABLED, allocs[i].attr)) {
			return i;
		}
	}
	assert(false && "too many objects, should be unreachable");
}

obj_tiles_handle obj_tiles_alloc(size_t num_tiles)
{
	assert(allocated < MAX_OBJECTS);
	assert(num_tiles != 0);
	size_t alloc_idx = find_disabled_alloc_idx();

	size_t start;
	size_t i = 0;
	while (true) {
		while (BIT_ARRAY_TEST(used_tiles, i)) {
			i++;
			if (i >= OBJ_TILES) {
				goto no_free_tiles_error;
			}
		}
		start = i;
		size_t count = 1;
		while (count < num_tiles) {
			i++;
			if (i >= OBJ_TILES) {
				goto no_free_tiles_error;
			}
			if (!BIT_ARRAY_TEST(used_tiles, i)) {
				count++;
			} else {
				break;
			}
		}
		if (count == num_tiles) {
			break;
		}
	}
	for (i = start; i < start + num_tiles; ++i) {
		BIT_ARRAY_SET(used_tiles, i);
	}

	struct object_tiles_alloc *alloc = &allocs[alloc_idx];
	alloc->start = start;
	alloc->attr = PREP(OBJ_TILES_COUNT, num_tiles) | OBJ_TILES_ENABLED;

	// Skip over zero on allocation so that { 0 } is never a valid handle
	if (alloc->generation == 0) {
		alloc->generation++;
	}

	obj_tiles_handle handle = { .index = alloc_idx,
				    .generation = alloc->generation };
	allocated++;
	return handle;

no_free_tiles_error:
	assert(false && "no block of free tiles big enough");
}

void obj_tiles_drop(obj_tiles_handle handle)
{
	assert(obj_tiles_exists(handle));
	struct object_tiles_alloc *alloc = &allocs[handle.index];
	size_t count = GET(OBJ_TILES_COUNT, alloc->attr);
	for (size_t i = alloc->start; i < alloc->start + count; ++i) {
		BIT_ARRAY_CLEAR(used_tiles, i);
	}
	alloc->generation++;
	alloc->start = 0;
	alloc->attr = 0;
	allocated--;
	return;
}

size_t obj_tiles_start(obj_tiles_handle handle)
{
	assert(obj_tiles_exists(handle));
	return allocs[handle.index].start;
}

size_t obj_tiles_count(obj_tiles_handle handle)
{
	assert(obj_tiles_exists(handle));
	return GET(OBJ_TILES_COUNT, allocs[handle.index].attr);
}

struct char_4bpp *nonnull obj_tiles_vram(obj_tiles_handle handle)
{
	assert(obj_tiles_exists(handle));
	size_t start = allocs[handle.index].start;
	return &char_block_begin(4)[start];
}

static_assert(sizeof(used_tiles) % 4 == 0, "needed for cpu_fast_fill/set");
static_assert(sizeof(allocs) % 4 == 0, "needed for cpu_fast_fill/set");

void obj_tiles_reset(void)
{
	allocated = 0;
	cpu_fast_fill(0, used_tiles, sizeof(used_tiles) / 4);
	cpu_fast_fill(0, allocs, sizeof(allocs) / 4);
}

static void test_drop_clear_tiles(struct nano_unit_case *nonnull test)
{
	obj_tiles_alloc(2);
	obj_tiles_handle h2 = obj_tiles_alloc(4);
	obj_tiles_alloc(2);
	obj_tiles_drop(h2);
	NANO_ASSERT(test, used_tiles[0] == 0xC3 /* 0b11000011 */, exit);
exit:
	obj_tiles_reset();
	return;
}

static void test_no_gaps(struct nano_unit_case *nonnull test)
{
	for (size_t i = 0; i < 8; ++i) {
		obj_tiles_alloc(1);
	}
	NANO_ASSERT(test, used_tiles[0] == 0xFF, exit);
exit:
	obj_tiles_reset();
	return;
}

static void test_find_hole(struct nano_unit_case *nonnull test)
{
	obj_tiles_handle h1 = obj_tiles_alloc(2);
	obj_tiles_handle h2 = obj_tiles_alloc(3);
	obj_tiles_handle h3 = obj_tiles_alloc(2);

	obj_tiles_drop(h2);
	for (size_t i = 0; i < 3; ++i) {
		obj_tiles_handle h = obj_tiles_alloc(1);
		NANO_ASSERT(test, obj_tiles_vram(h1) < obj_tiles_vram(h), exit);
		NANO_ASSERT(test, obj_tiles_vram(h) < obj_tiles_vram(h3), exit);
	}

	obj_tiles_handle hl = obj_tiles_alloc(1);
	NANO_ASSERT(test, obj_tiles_vram(hl) > obj_tiles_vram(h3), exit);
exit:
	obj_tiles_reset();
	return;
}

struct nano_unit_case obj_tiles_test_suite[] = { NANO_UNIT_CASE(
							 test_drop_clear_tiles),
						 NANO_UNIT_CASE(test_no_gaps),
						 NANO_UNIT_CASE(test_find_hole),
						 {} };