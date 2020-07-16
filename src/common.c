#include "chickpea/common.h"

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