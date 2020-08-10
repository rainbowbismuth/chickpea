#ifndef CHICKPEA_VEC2_H
#define CHICKPEA_VEC2_H

#include "chickpea.h"

struct vec2 {
	int16_t x;
	int16_t y;
};

static inline struct vec2 v2_add_x(struct vec2 pos, int16_t x)
{
	pos.x += x;
	return pos;
}

static inline struct vec2 v2_add_xy(struct vec2 pos, int16_t x, int16_t y)
{
	pos.x += x;
	pos.y += y;
	return pos;
}

#endif // CHICKPEA_VEC2_H
