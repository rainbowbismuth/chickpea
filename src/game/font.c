#include "game/font.h"

void text_renderer_init(struct text_renderer *nonnull renderer,
			const struct font *nonnull font,
			const struct text_settings *nonnull settings,
			const char *nonnull message)
{
	renderer->font = font;
	renderer->message = message;
	renderer->settings = settings;
	renderer->chars = settings->chars;
	renderer->screen = settings->screen;
	renderer->i = 0;
}

static struct char_4bpp *nonnull
lookup_char(struct text_renderer *nonnull renderer, uint32_t ch)
{
	if (renderer->font->tall) {
		return &renderer->font->characters[ch * 2];
	} else {
		return &renderer->font->characters[ch];
	}
}

static struct char_4bpp *nonnull
out_char(struct text_renderer *nonnull renderer, uint32_t index)
{
	if (renderer->font->tall) {
		return (struct char_4bpp *)&renderer->chars[index * 2];
	} else {
		return (struct char_4bpp *)&renderer->chars[index];
	}
}

static void add_to_screen(const struct text_renderer *renderer,
			  volatile struct char_4bpp *chars,
			  volatile uint16_t *screen)
{
	uint16_t pal = PREP(TILE_PALETTE, renderer->settings->palette);
	*screen = PREP(TILE_CHAR,
		       char_name(renderer->settings->char_block, chars))
		| pal;
	if (renderer->font->tall) {
		*(screen + 32) = PREP(TILE_CHAR,
				      char_name(renderer->settings->char_block,
						chars + 1))
			       | pal;
	}
}

static uint32_t char_width(struct text_renderer *nonnull renderer, uint8_t ch)
{
	if (ch == ' ') {
		return 4;
	}
	if (renderer->font->widths) {
		return renderer->font->widths[ch];
	}
	return 8;
}

bool text_renderer_next_char(struct text_renderer *nonnull renderer)
{
	uint8_t ch = *renderer->message;
	if (ch == '\0') {
		return false;
	}
	renderer->message++;
	uint32_t width = char_width(renderer, ch);
	uint32_t idx = renderer->i / 8;
	uint32_t end_idx = (renderer->i + width) / 8;
	uint32_t offset = renderer->i % 8;
	struct char_4bpp *ch_gfx = lookup_char(renderer, ch);
	struct char_4bpp *out = out_char(renderer, idx);
	ch4bpp_bitor_shr(out, ch_gfx, offset);
	if (renderer->font->tall) {
		ch4bpp_bitor_shr(out + 1, ch_gfx + 1, offset);
	}
	if (idx != end_idx && offset != 0) {
		offset = 8 - offset;
		out = out_char(renderer, end_idx);
		ch4bpp_bitor_shl(out, ch_gfx, offset);
		if (renderer->font->tall) {
			ch4bpp_bitor_shl(out + 1, ch_gfx + 1, offset);
		}
	}
	renderer->i += width + renderer->font->letter_spacing;
	return true;
}

void text_render(const struct font *nonnull font,
		 const struct text_settings *nonnull settings,
		 const char *nonnull message)
{
	struct text_renderer renderer = { 0 };
	text_renderer_init(&renderer, font, settings, message);

	/*
	 * FIXME: Temporarily cheating here...
	 */
	for (size_t i = 0; i < 20; ++i) {
		add_to_screen(
			&renderer,
			&renderer.chars[i * (renderer.font->tall ? 2 : 1)],
			&renderer.screen[i]);
	}

	while (text_renderer_next_char(&renderer)) {
		;
	}
}