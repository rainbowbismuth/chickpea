#include "game/font.h"

void text_renderer_init(struct text_renderer *nonnull renderer,
			const struct font *nonnull font,
			const struct text_config *nonnull config,
			const char *nonnull message)
{
	renderer->font = font;
	renderer->message = message;
	renderer->original_message = message;
	renderer->config = config;
	renderer->chars = config->chars;
	renderer->screen = config->screen;
	renderer->gfx_i = 0;
	renderer->clear_idx = 0;
	renderer->screen_i = 0;
}

static struct char_4bpp *nonnull
lookup_char(const struct text_renderer *nonnull renderer, uint32_t ch)
{
	if (renderer->font->tall) {
		return &renderer->font->characters[ch * 2];
	} else {
		return &renderer->font->characters[ch];
	}
}

static struct char_4bpp *nonnull
out_char(const struct text_renderer *nonnull renderer, uint32_t index)
{
	if (renderer->font->tall) {
		return (struct char_4bpp *)&renderer->chars[index * 2];
	} else {
		return (struct char_4bpp *)&renderer->chars[index];
	}
}

static void add_to_screen(const struct text_renderer *renderer,
			  const struct char_4bpp *ch, uint16_t *screen)
{
	uint16_t pal = PREP(TILE_PALETTE, renderer->config->palette);
	*screen = PREP(TILE_CHAR, char_name(renderer->config->char_block, ch))
		| pal;
	if (renderer->font->tall) {
		*(screen + 32) =
			PREP(TILE_CHAR,
			     char_name(renderer->config->char_block, ch + 1))
			| pal;
	}
}

static uint32_t char_width(struct text_renderer *nonnull renderer, uint8_t ch)
{
	if (renderer->font->widths) {
		return renderer->font->widths[ch];
	}
	return 8;
}

bool text_renderer_at_end(struct text_renderer *nonnull renderer)
{
	if (*renderer->message == '\0') {
		return true;
	}
	return false;
}

static void render_normal_char(struct text_renderer *nonnull renderer)
{
	uint8_t ch = *renderer->message;
	renderer->message++;
	uint32_t width = char_width(renderer, ch);
	uint32_t gfx_idx = renderer->gfx_i / 8;
	uint32_t screen_idx = renderer->screen_i / 8;
	uint32_t gfx_end_idx = (renderer->gfx_i + width) / 8;
	uint32_t screen_end_idx = (renderer->screen_i + width) / 8;
	uint32_t gfx_offset = renderer->gfx_i % 8;

	while (renderer->clear_idx <= gfx_end_idx) {
		struct char_4bpp *clear_ch =
			out_char(renderer, renderer->clear_idx);
		cpu_fast_fill(0, clear_ch,
			      sizeof(*clear_ch)
				      / (renderer->font->tall ? 2 : 4));
		renderer->clear_idx++;
	}

	struct char_4bpp *ch_gfx = lookup_char(renderer, ch);
	struct char_4bpp *out = out_char(renderer, gfx_idx);
	ch4bpp_bitor_shr(out, ch_gfx, gfx_offset);
	if (renderer->font->tall) {
		ch4bpp_bitor_shr(out + 1, ch_gfx + 1, gfx_offset);
	}
	add_to_screen(renderer, out_char(renderer, gfx_idx),
		      &renderer->screen[screen_idx]);

	if (gfx_idx != gfx_end_idx && gfx_offset != 0) {
		gfx_offset = 8 - gfx_offset;
		out = out_char(renderer, gfx_end_idx);
		ch4bpp_bitor_shl(out, ch_gfx, gfx_offset);
		if (renderer->font->tall) {
			ch4bpp_bitor_shl(out + 1, ch_gfx + 1, gfx_offset);
		}
		add_to_screen(renderer, out_char(renderer, gfx_end_idx),
			      &renderer->screen[screen_end_idx]);
	}
	renderer->gfx_i += width + renderer->font->letter_spacing;
	renderer->screen_i += width + renderer->font->letter_spacing;
}

bool text_renderer_next_char(struct text_renderer *nonnull renderer)
{
	uint8_t ch = *renderer->message;
	if (ch == '\n') {
		renderer->screen += renderer->font->tall ? 64 : 32;
		renderer->screen_i = 0;
		renderer->gfx_i = (renderer->gfx_i + 8) & ~0x7;
		renderer->message++;
		ch = *renderer->message;
	}
	if (ch == '\0') {
		return false;
	} else if (ch == ' ') {
		// TODO: Remove hardcoded space size.
		renderer->screen_i += 4;
		renderer->gfx_i += 4;
		renderer->message++;
	} else if (ch == '\06') {
		renderer->message++;
		return true;
	}
	render_normal_char(renderer);
	return true;
}

void text_renderer_clear(struct text_renderer *nonnull renderer)
{
	renderer->message = renderer->original_message;
	renderer->screen = renderer->config->screen;
	renderer->gfx_i = 0;
	renderer->clear_idx = 0;
	renderer->screen_i = 0;
	cpu_fast_fill(0, renderer->screen, sizeof(struct screen) / 4);
}

void text_render(const struct font *nonnull font,
		 const struct text_config *nonnull config,
		 const char *nonnull message)
{
	struct text_renderer renderer = { 0 };
	text_renderer_init(&renderer, font, config, message);

	while (text_renderer_next_char(&renderer)) {
		;
	}
}
