#ifndef CHICKPEA_DEBUG_FONT_H
#define CHICKPEA_DEBUG_FONT_H

#include "chickpea.h"

struct debug_font {
	struct palette palette;
	struct character_4bpp characters[256];
};

void write_debug_msg(const struct debug_font *font, uint32_t char_block,
		     uint32_t screen_block, uint32_t palette, uint32_t tile_x,
		     uint32_t tile_y, const char *msg);

extern const struct debug_font default_debug_font;

#endif //CHICKPEA_C_DEBUG_FONT_H
