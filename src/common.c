#include "chickpea/common.h"
#include "chickpea/nano_unit.h"

uint32_t reverse_nibbles(uint32_t n)
{
	n = (n & 0x0F0F0F0F) << 4 | (n & 0xF0F0F0F0) >> 4;
	n = (n & 0x00FF00FF) << 8 | (n & 0xFF00FF00) >> 8;
	n = (n & 0x0000FFFF) << 16 | (n & 0xFFFF0000) >> 16;
	return n;
}

void write_4bpp(const struct character_4bpp *src,
		volatile struct character_4bpp *dst)
{
	for (size_t i = 0; i < ARRAY_SIZE(src->lines); ++i) {
		dst->lines[i] = src->lines[i];
	}
}

void write_palette(const struct palette *src, volatile struct palette *dst)
{
	for (size_t i = 0; i < ARRAY_SIZE(src->color); ++i) {
		dst->color[i] = src->color[i];
	}
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

void test_reverse_nibbles(struct nano_unit_case *test)
{
	uint32_t x = 0x2d8fa90a;
	NANO_ASSERT(test, x == reverse_nibbles(reverse_nibbles(x)), exit);
exit:
	return;
}

void test_additive_blend(struct nano_unit_case *test)
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

struct nano_unit_case common_test_suite[] = {
	NANO_UNIT_CASE(test_reverse_nibbles),
	NANO_UNIT_CASE(test_additive_blend),
	{}
};