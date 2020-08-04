#include "chickpea/common.h"
#include "chickpea/nano_unit.h"
#include "chickpea.h"

uint32_t char_name(uint32_t char_block,
		   volatile struct char_4bpp *nonnull character)
{
	volatile struct char_4bpp *start = char_block_begin(char_block);
	uint32_t ret = character - start;
	assert(ret < 1024);
	return ret;
}

uint32_t reverse_nibbles(uint32_t n)
{
	n = (n & 0x0F0F0F0F) << 4 | (n & 0xF0F0F0F0) >> 4;
	n = (n & 0x00FF00FF) << 8 | (n & 0xFF00FF00) >> 8;
	n = (n & 0x0000FFFF) << 16 | (n & 0xFFFF0000) >> 16;
	return n;
}

void write_4bpp(const struct char_4bpp *restrict nonnull src,
		volatile struct char_4bpp *restrict nonnull dst)
{
	cpu_fast_set(src, (void *)dst, sizeof(*src) / 4);
}

void write_4bpp_n(const struct char_4bpp *restrict nonnull src,
		  volatile struct char_4bpp *restrict nonnull dst, size_t n)
{
	cpu_fast_set(src, (void *)dst, (sizeof(*src) * n) / 4);
}

void write_palette(const struct palette *restrict nonnull src,
		   volatile struct palette *restrict nonnull dst)
{
	cpu_fast_set(src, (void *)dst, sizeof(*src) / 4);
}

uint16_t color(uint32_t red, uint32_t green, uint32_t blue)
{
	return PREP(COL_RED, red) | PREP(COL_GREEN, green) |
	       PREP(COL_BLUE, blue);
}

uint16_t additive_blend(uint16_t src_color, uint16_t src_weight,
			uint16_t dst_color, uint16_t dst_weight)
{
	src_weight = src_weight > 16 ? 16 : src_weight;
	dst_weight = dst_weight > 16 ? 16 : dst_weight;
	uint32_t red = (GET(COL_RED, src_color) * src_weight +
			GET(COL_RED, dst_color) * dst_weight) /
		       16;
	red = red > 31 ? 31 : red;
	uint32_t green = (GET(COL_GREEN, src_color) * src_weight +
			  GET(COL_GREEN, dst_color) * dst_weight) /
			 16;
	green = green > 31 ? 31 : green;
	uint32_t blue = (GET(COL_BLUE, src_color) * src_weight +
			 GET(COL_BLUE, dst_color) * dst_weight) /
			16;
	blue = blue > 31 ? 31 : blue;
	return PREP(COL_RED, red) | PREP(COL_GREEN, green) |
	       PREP(COL_BLUE, blue);
}

static uint8_t width_table[4][3] = {
	/* Square | Horizontal | Vertical */
	{ 1, 2, 1 },
	{ 2, 4, 1 },
	{ 4, 4, 2 },
	{ 8, 8, 4 }
};

size_t object_width(enum obj_shape shape, enum obj_size size)
{
	return width_table[size][shape];
}

static uint8_t height_table[4][3] = {
	/* Square | Horizontal | Vertical */
	{ 1, 1, 2 },
	{ 2, 1, 4 },
	{ 4, 2, 4 },
	{ 8, 4, 8 }
};

size_t object_height(enum obj_shape shape, enum obj_size size)
{
	return height_table[size][shape];
}

static uint8_t size_table[4][3] = {
	/* Square | Horizontal | Vertical */
	{ 1, 2, 2 },
	{ 4, 4, 4 },
	{ 16, 8, 8 },
	{ 64, 32, 32 },
};

size_t tiles_in_object(enum obj_shape shape, enum obj_size size)
{
	return size_table[size][shape];
}

void char_4bpp_bitwise_or(struct char_4bpp *restrict nonnull self,
			  const struct char_4bpp *restrict nonnull other)
{
	for (size_t i = 0; i < ARRAY_SIZE(self->lines); ++i) {
		self->lines[i] |= other->lines[i];
	}
}

void char_4bpp_shift_left(struct char_4bpp *nonnull self, uint32_t amount)
{
	if (amount == 0) {
		return;
	}
	amount *= 4;
	for (size_t i = 0; i < ARRAY_SIZE(self->lines); ++i) {
		self->lines[i] >>= amount;
	}
}

void char_4bpp_shift_right(struct char_4bpp *nonnull self, uint32_t amount)
{
	if (amount == 0) {
		return;
	}
	amount *= 4;
	for (size_t i = 0; i < ARRAY_SIZE(self->lines); ++i) {
		self->lines[i] <<= amount;
	}
}

void char_4bpp_flip_vertical(struct char_4bpp *nonnull self)
{
	struct char_4bpp copy = *self;
	for (size_t i = 0; i < ARRAY_SIZE(self->lines); ++i) {
		self->lines[i] = copy.lines[7 - i];
	}
}

void char_4bpp_flip_horizontal(struct char_4bpp *nonnull self)
{
	for (size_t i = 0; i < ARRAY_SIZE(self->lines); ++i) {
		self->lines[i] = reverse_nibbles(self->lines[i]);
	}
}

void char_4bpp_flip_both(struct char_4bpp *nonnull self)
{
	char_4bpp_flip_vertical(self);
	char_4bpp_flip_horizontal(self);
}

void debug_put_u32(uint32_t n)
{
	char digits[12] = { 0 };
	size_t i;
	for (i = 10; i != 0 && n >= 10; i--) {
		digits[i] = '0' + (n % 10);
		n /= 10;
	}
	digits[i] = '0' + (n % 10);
	debug_put_str(&digits[i]);
}

static void test_reverse_nibbles(struct nano_unit_case *nonnull test)
{
	uint32_t x = 0x2d8fa90a;
	NANO_ASSERT(test, x == reverse_nibbles(reverse_nibbles(x)), exit);
exit:
	return;
}

static void test_additive_blend(struct nano_unit_case *nonnull test)
{
	uint16_t red = color(30, 0, 0);
	uint16_t blue = color(0, 0, 30);

	uint16_t all_red = additive_blend(red, 16, blue, 0);
	uint16_t all_blue = additive_blend(red, 0, blue, 16);
	NANO_ASSERT(test, red == all_red, exit);
	NANO_ASSERT(test, blue == all_blue, exit);

	uint16_t purple = additive_blend(red, 8, blue, 8);
	NANO_ASSERT(test, purple == color(15, 0, 15), exit);
exit:
	return;
}

static void test_cpu_fast_fill(struct nano_unit_case *nonnull test)
{
	uint32_t buffer[8] = { 0 };
	uint32_t pattern = 0x101FA0AB;

	cpu_fast_fill(pattern, &buffer, ARRAY_SIZE(buffer));
	for (size_t i = 0; i < ARRAY_SIZE(buffer); ++i) {
		NANO_ASSERT(test, buffer[i] == pattern, exit);
	}
exit:
	return;
}

static void test_object_sizes(struct nano_unit_case *nonnull test)
{
	enum obj_shape shapes[3] = { OBJ_SHAPE_SQUARE, OBJ_SHAPE_VERTICAL,
				     OBJ_SHAPE_VERTICAL };
	enum obj_size sizes[4] = { OBJ_SIZE_8, OBJ_SIZE_16, OBJ_SIZE_32,
				   OBJ_SIZE_64 };

	for (size_t i = 0; i < ARRAY_SIZE(shapes); ++i) {
		for (size_t j = 0; j < ARRAY_SIZE(sizes); ++j) {
			enum obj_shape shape = shapes[i];
			enum obj_size size = sizes[j];

			size_t width = object_width(shape, size);
			size_t height = object_height(shape, size);
			size_t total = tiles_in_object(shape, size);
			NANO_ASSERT(test, total == width * height, exit);
		}
	}
exit:
	return;
}

static void test_char_name(struct nano_unit_case *nonnull test)
{
	volatile struct char_4bpp *ch = char_block_begin(1);
	volatile struct char_4bpp *ch5 = ch + 5;
	NANO_ASSERT(test, char_name(1, ch5) == 5, exit);
exit:
	return;
}

struct nano_unit_case common_test_suite[] = {
	NANO_UNIT_CASE(test_reverse_nibbles),
	NANO_UNIT_CASE(test_additive_blend),
	NANO_UNIT_CASE(test_cpu_fast_fill),
	NANO_UNIT_CASE(test_object_sizes),
	NANO_UNIT_CASE(test_char_name),
	{}
};