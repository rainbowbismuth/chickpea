#ifndef CHICKPEA_TEXT_BOX_H
#define CHICKPEA_TEXT_BOX_H

#include "chickpea.h"

struct text_box_graphics {
	const struct char_4bpp *chars;
	const uint16_t *tiles;
	uint8_t height;
	uint8_t width;
};

struct text_box_settings {
	const struct text_box_graphics *gfx;
	uint8_t char_block;
	uint8_t screen_block;
	uint8_t from_top;
	uint8_t width;
	uint8_t palette;
	bool align_right;
};

void text_box_draw(const struct text_box_settings *settings);

#endif //CHICKPEA_TEXT_BOX_H
