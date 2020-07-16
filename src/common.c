#include "chickpea/common.h"

uint32_t reverse_nibbles(uint32_t n)
{
	n = (n & 0x0F0F0F0F) << 4 | (n & 0xF0F0F0F0) >> 4;
	n = (n & 0x00FF00FF) << 8 | (n & 0xFF00FF00) >> 8;
	n = (n & 0x0000FFFF) << 16 | (n & 0xFFFF0000) >> 16;
	return n;
}

#ifdef BUILD_GBA
static void memcpy16(const uint16_t *src, size_t length, volatile uint16_t *dst)
{
	while (length--) {
		*dst++ = *src++;
	}
}
#endif

void write_4bpp(const struct character_4bpp *src,
		volatile struct character_4bpp *dst)
{
#ifdef BUILD_GBA
	memcpy16((const uint16_t *)&src->lines, ARRAY_SIZE(src->lines) * 2,
		 (volatile uint16_t *)&dst->lines);
#else
	for (size_t i = 0; i < ARRAY_SIZE(src->lines); ++i) {
		dst->lines[i] = src->lines[i];
	}
#endif
}

void write_palette(const struct palette *src, volatile struct palette *dst)
{
#ifdef BUILD_GBA
	memcpy16(&src->color[0], ARRAY_SIZE(src->color), &dst->color[0]);
#else
	for (size_t i = 0; i < ARRAY_SIZE(src->color); ++i) {
		dst->color[i] = src->color[i];
	}
#endif
}