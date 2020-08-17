#ifndef CHICKPEA_FONT_H
#define CHICKPEA_FONT_H

#include "chickpea.h"
#include "chickpea/vec2.h"

struct font {
	struct char_4bpp *nonnull characters;
	uint8_t *nullable widths;
	uint8_t letter_spacing;
	bool tall;
};

struct text_config {
	struct char_4bpp *nonnull chars;
	uint16_t *nonnull screen;
	uint8_t char_block;
	uint8_t palette;
};

struct text_renderer {
	const struct font *nonnull font;
	const struct text_config *nonnull config;

	struct char_4bpp *nonnull chars;
	uint16_t *nonnull screen;
	const char *nonnull message;
	uint32_t i;
};

void text_renderer_init(struct text_renderer *nonnull renderer,
			const struct font *nonnull font,
			const struct text_config *nonnull config,
			const char *nonnull message);

/*
 * Renders the next character in the text_renderer, returns false
 * if there are no more characters to render.
 */
bool text_renderer_next_char(struct text_renderer *nonnull renderer);

/*
 * Render an entire message.
 */
void text_render(const struct font *nonnull font,
		 const struct text_config *nonnull config,
		 const char *nonnull message);

#endif // CHICKPEA_FONT_H