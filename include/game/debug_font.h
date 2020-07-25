#ifndef CHICKPEA_DEBUG_FONT_H
#define CHICKPEA_DEBUG_FONT_H

#include "chickpea.h"

struct debug_font {
	const struct palette *nonnull palette;
	const struct char_4bpp *nonnull characters;
};

void write_debug_msg(const struct debug_font *nonnull font, uint32_t char_block,
		     uint32_t screen_block, uint32_t palette, uint32_t tile_x,
		     uint32_t tile_y, const char *nonnull msg);

extern const struct char_4bpp debug_font_4bpp[256];
extern const struct palette debug_font_pal;

#endif //CHICKPEA_DEBUG_FONT_H
