#ifndef CHICKPEA_DEBUG_FONT_H
#define CHICKPEA_DEBUG_FONT_H

#include "chickpea.h"
#include "sprite.h"

struct debug_font {
	const struct palette *nonnull palette;
	const struct char_4bpp *nonnull characters;
};

void write_debug_msg(const struct debug_font *nonnull font, uint32_t char_block,
		     uint32_t screen_block, uint32_t palette, uint32_t tile_x,
		     uint32_t tile_y, const char *nonnull msg);

sprite_handle
write_debug_msg_sprite(const struct debug_font *nonnull font,
		       const struct sprite_template *nonnull template,
		       const char *nonnull msg);

extern struct resource fonts_debug_font_4bpp;
extern struct resource fonts_debug_font_pal;

#endif // CHICKPEA_DEBUG_FONT_H
