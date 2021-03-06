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
	renderer->gfx_px = 0;
	renderer->clear_tile = 0;
	renderer->screen_px = 0;
}

static struct char_4bpp *nonnull
lookup_char(const struct text_renderer *nonnull renderer, uint32_t ch)
{
	return &renderer->font->characters[ch << renderer->font->tall];
}

static struct char_4bpp *nonnull
out_char(const struct text_renderer *nonnull renderer, uint32_t index)
{
	return &renderer->chars[index << renderer->font->tall];
}

static void add_to_screen(const struct text_renderer *renderer,
			  const struct char_4bpp *ch, uint16_t *screen)
{
	uint16_t pal = PREP(TILE_PALETTE, renderer->config->palette);
	*screen = PREP(TILE_CHAR, char_name(renderer->config->char_block, ch))
		| pal;
	if (renderer->font->tall) {
		screen += 32;
		*screen = PREP(TILE_CHAR,
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
	uint32_t gfx_tile = renderer->gfx_px / 8;
	uint32_t screen_tile = renderer->screen_px / 8;
	uint32_t gfx_end_tile = (renderer->gfx_px + width) / 8;
	uint32_t screen_end_tile = (renderer->screen_px + width) / 8;
	uint32_t gfx_px_offset = renderer->gfx_px % 8;

	while (renderer->clear_tile <= gfx_end_tile) {
		struct char_4bpp *clear_ch =
			out_char(renderer, renderer->clear_tile);
		cpu_fast_fill(0, clear_ch,
			      sizeof(*clear_ch) / (4 >> renderer->font->tall));
		renderer->clear_tile++;
	}

	struct char_4bpp *ch_gfx = lookup_char(renderer, ch);
	struct char_4bpp *out = out_char(renderer, gfx_tile);
	ch4bpp_bitor_shr(out, ch_gfx, gfx_px_offset);
	if (renderer->font->tall) {
		ch4bpp_bitor_shr(out + 1, ch_gfx + 1, gfx_px_offset);
	}
	add_to_screen(renderer, out, &renderer->screen[screen_tile]);

	if (gfx_tile != gfx_end_tile && gfx_px_offset != 0) {
		gfx_px_offset = 8 - gfx_px_offset;
		out = out_char(renderer, gfx_end_tile);
		ch4bpp_bitor_shl(out, ch_gfx, gfx_px_offset);
		if (renderer->font->tall) {
			ch4bpp_bitor_shl(out + 1, ch_gfx + 1, gfx_px_offset);
		}
		add_to_screen(renderer, out,
			      &renderer->screen[screen_end_tile]);
	}
	renderer->gfx_px += width + renderer->font->letter_spacing;
	renderer->screen_px += width + renderer->font->letter_spacing;
}

bool text_renderer_next_char(struct text_renderer *nonnull renderer)
{
	uint8_t ch = *renderer->message;
	if (ch == '\n') {
		renderer->screen += 32 << renderer->font->tall;
		renderer->screen_px = 0;
		renderer->gfx_px = (renderer->gfx_px + 8) & ~0x7;
		renderer->message++;
		ch = *renderer->message;
	}
	if (ch == '\0') {
		return false;
	} else if (ch == ' ') {
		// TODO: Remove hardcoded space size.
		renderer->screen_px += 8 >> renderer->font->tall;
		renderer->gfx_px += 8 >> renderer->font->tall;
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
	renderer->gfx_px = 0;
	renderer->clear_tile = 0;
	renderer->screen_px = 0;
	cpu_fast_fill(0,
		      (uint16_t *)((uintptr_t)renderer->screen
				   & ~(sizeof(struct screen) - 1)),
		      sizeof(struct screen) / 4);
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
