#include "game/sprite.h"
#include "game/object_tiles.h"

struct sprite_priv {
	struct sprite pub;
	uint8_t generation;
	bool used;
	struct sprite_template template;
	struct vec2 center;
	obj_tiles_handle tile_handles[MAX_SPRITE_OBJECTS];
};

static size_t allocated = 0;
static struct sprite_priv sprites[MAX_SPRITES] = { 0 };
static uint8_t sorted_sprites[MAX_SPRITES] = { 0 };

static size_t objs_in_buf = 0;
static struct object_attribute_mem oam_buf = { 0 };

size_t sprite_allocated(void)
{
	return allocated;
}

bool sprite_exists(sprite_handle handle)
{
	assert(handle.index < MAX_SPRITES);
	return sprites[handle.index].generation == handle.generation &&
	       sprites[handle.index].used;
}

struct sprite *nonnull sprite_ref(sprite_handle handle)
{
	assert(sprite_exists(handle));
	return &sprites[handle.index].pub;
}

static size_t find_first_unused(void)
{
	for (size_t i = 0; i < MAX_SPRITES; ++i) {
		if (!sprites[i].used) {
			return i;
		}
	}
	assert(false && "no disabled sprites");
}

static struct vec2 calculate_center(const struct sprite_priv *nonnull sprite)
{
	int32_t min_x = 0xFFFF, min_y = 0xFFFF;
	int32_t max_x = -0xFFFF, max_y = -0xFFFF;
	for (size_t i = 0; i < sprite->template.num_objects; ++i) {
		const struct sprite_object_def *obj_def =
			&sprite->template.objects[i];

		if (obj_def->x_offset < min_x) {
			min_x = obj_def->x_offset;
		}
		if (obj_def->y_offset < min_y) {
			min_y = obj_def->y_offset;
		}
		int32_t x_extent = obj_def->x_offset +
				   object_width(obj_def->shape, obj_def->size);
		if (x_extent > max_x) {
			max_x = x_extent;
		}
		int32_t y_extent = obj_def->y_offset +
				   object_height(obj_def->shape, obj_def->size);
		if (y_extent > max_y) {
			max_y = y_extent;
		}
	}
	return (struct vec2){ .x = (min_x + max_x) / 2,
			      .y = (min_y + max_y) / 2 };
}

sprite_handle sprite_alloc(const struct sprite_template *nonnull template)
{
	assert(allocated < MAX_SPRITES);
	assert(template->objects != NULL);
	assert(template->num_objects < MAX_SPRITE_OBJECTS);

	size_t index = find_first_unused();
	struct sprite_priv *sprite = &sprites[index];
	sprite_handle handle = { .index = index,
				 .generation = sprite->generation };
	sprite->used = true;
	cpu_fast_fill(0, &sprite->pub, sizeof(sprite->pub) / 4);
	sprite->template = *template;
	sprite->pub.palette = template->palette;
	sprite->pub.mode = template->mode;
	sprite->center = calculate_center(sprite);

	for (size_t i = 0; i < template->num_objects; ++i) {
		const struct sprite_object_def *obj_def = &template->objects[i];
		size_t tiles = tiles_in_object(obj_def->shape, obj_def->size);
		obj_tiles_handle obj_h = obj_tiles_alloc(tiles);
		sprite->tile_handles[i] = obj_h;
	}

	allocated++;
	return handle;
}

volatile struct char_4bpp *nonnull sprite_obj_vram(sprite_handle handle,
						   size_t idx)
{
	assert(sprite_exists(handle));
	struct sprite_priv *sprite = &sprites[handle.index];
	assert(idx < sprite->template.num_objects);
	return obj_tiles_vram(sprite->tile_handles[idx]);
}

void sprite_drop(sprite_handle handle)
{
	assert(sprite_exists(handle));
	struct sprite_priv *sprite = &sprites[handle.index];
	sprite->generation++;
	sprite->used = false;
	for (size_t i = 0; i < sprites->template.num_objects; ++i) {
		obj_tiles_drop(sprite->tile_handles[i]);
	}
}

static void add_object_to_buffer(const struct sprite_priv *nonnull sprite,
				 const struct sprite_object_def *nonnull object,
				 size_t tile_start, size_t priority)
{
	struct oam_entry *entry = &oam_buf.entries[objs_in_buf];

	int32_t x_offset = object->x_offset;
	int32_t y_offset = object->y_offset;
	uint32_t flip_bits = 0;
	if (sprite->pub.flip_horizontal) {
		flip_bits |= OBJA1_HORIZONTAL_FLIP;
		x_offset = -(x_offset - sprite->center.x) + sprite->center.x;
	}
	if (sprite->pub.flip_vertical) {
		flip_bits |= OBJA1_VERTICAL_FLIP;
		y_offset = -(y_offset - sprite->center.y) + sprite->center.y;
	}
	struct vec2 pos = v2_add_xy(sprite->pub.pos, x_offset, y_offset);

	entry->attr_0 = PREP(OBJA0_Y, pos.y) |
			PREP(OBJA0_MODE, sprite->pub.mode) |
			PREP(OBJA0_SHAPE, object->shape);
	entry->attr_1 = PREP(OBJA1_X, pos.x) | PREP(OBJA1_SIZE, object->size) |
			flip_bits;
	entry->attr_2 = PREP(OBJA2_CHAR, tile_start) |
			PREP(OBJA2_PALETTE, sprite->pub.palette) |
			PREP(OBJA2_PRIORITY, priority);
	entry->_rotation_scaling_padding = 0;

	objs_in_buf++;
}

static void add_sprite_to_buffer(const struct sprite_priv *nonnull sprite)
{
	if (!sprite->pub.enabled) {
		return;
	}

	assert(sprite->template.num_objects + objs_in_buf <
	       ARRAY_SIZE(oam_buf.entries));

	for (size_t i = 0; i < sprite->template.num_objects; ++i) {
		size_t start = obj_tiles_start(sprite->tile_handles[i]);
		size_t priority = sprite->pub.priority[i];
		add_object_to_buffer(sprite, &sprite->template.objects[i],
				     start, priority);
	}
}

static void sort_sprites_by_priority(void)
{
	size_t sprite_n = 0;
	for (size_t i = 0; i < ARRAY_SIZE(sprites); ++i) {
		if (!sprites[i].used) {
			continue;
		}
		sorted_sprites[sprite_n] = i;
		sprite_n++;
	}
	assert(sprite_n == allocated);
	for (size_t i = 1; i < sprite_n; ++i) {
		size_t j = i;
		while (j > 0 && sprites[sorted_sprites[j - 1]].pub.order >
					sprites[sorted_sprites[j]].pub.order) {
			uint8_t tmp = sorted_sprites[j];
			sorted_sprites[j] = sorted_sprites[j - 1];
			sorted_sprites[j - 1] = tmp;
			j--;
		}
	}
}

static_assert(sizeof(struct sprite) % 4 == 0, "needed for cpu_fast_fill/set");
static_assert(sizeof(sprites) % 4 == 0, "needed for cpu_fast_fill/set");
static_assert(sizeof(sorted_sprites) % 4 == 0, "needed for cpu_fast_fill/set");
static_assert(sizeof(oam_buf) % 4 == 0, "needed for cpu_fast_fill/set");

void sprite_reset(void)
{
	allocated = 0;
	objs_in_buf = 0;
	cpu_fast_fill(0, &sprites, sizeof(sprites) / 4);
	cpu_fast_fill(0, &sorted_sprites, sizeof(sorted_sprites) / 4);
	cpu_fast_fill(0, &oam_buf, sizeof(oam_buf) / 4);
	obj_tiles_reset();
}

void sprite_build_oam_buffer(void)
{
	objs_in_buf = 0;
	sort_sprites_by_priority();

	for (size_t i = 0; i < allocated; ++i) {
		add_sprite_to_buffer(&sprites[sorted_sprites[i]]);
	}

	for (size_t i = objs_in_buf; i < ARRAY_SIZE(oam_buf.entries); ++i) {
		oam_buf.entries[i].attr_0 |= OBJA0_OBJ_DISABLE;
	}
}

void sprite_commit_buffer_to_oam(void)
{
	cpu_fast_set(&oam_buf.entries, (void *)&OAM,
		     sizeof(oam_buf.entries) / 4);
}
