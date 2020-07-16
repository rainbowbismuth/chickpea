#include <stdbool.h>
#include "chickpea.h"
#include "SDL.h"
#include "SDL_timer.h"

uint16_t reg_dispcnt = 0;
uint16_t reg_dispstat = 0;
uint16_t reg_vcount = 0;
uint32_t reg_ime = 0;
uint16_t reg_ie = 0;
uint16_t reg_if = 0;

uint16_t reg_bg_controls[4] = { 0 };
uint16_t reg_bg_scrolls_x[4] = { 0 };
uint16_t reg_bg_scrolls_y[4] = { 0 };

uint8_t video_ram[0x18000] = { 0 };

struct palette bg_pallete_ram[16] = { { 0 } };
struct palette obj_pallete_ram[16] = { { 0 } };

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Surface *surface = NULL;
SDL_Texture *texture = NULL;

uint32_t ticks_previous = 0;
uint32_t ticks_lag = 0;
uint64_t frame_counter = 0;

uint32_t real_win_width = GBA_WIDTH * 2;
uint32_t real_win_height = GBA_HEIGHT * 2;

uint16_t screen_color[GBA_HEIGHT][GBA_WIDTH + 16] = { 0 };
uint16_t screen_priority[GBA_HEIGHT][GBA_WIDTH + 16] = { 0 };

int main(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);
	window = SDL_CreateWindow("chickpea-c", SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED, real_win_width,
				  real_win_height, SDL_WINDOW_ALLOW_HIGHDPI);
	assert(window != NULL);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	assert(renderer != NULL);

	surface = SDL_CreateRGBSurfaceWithFormat(0, GBA_WIDTH, GBA_HEIGHT, 16,
						 SDL_PIXELFORMAT_RGB555);
	assert(surface != NULL);

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB555,
				    SDL_TEXTUREACCESS_STREAMING, GBA_WIDTH,
				    GBA_HEIGHT);
	assert(texture != NULL);

	game_main();
}

void clear_line(uint16_t bg_color, uint16_t y)
{
	for (size_t i = 0; i < ARRAY_SIZE(screen_color[0]); ++i) {
		screen_color[y][i] = bg_color;
		screen_priority[y][i] = 0xFFFF;
	}
}

void draw_pixel(uint32_t x, uint32_t y, uint16_t color, uint16_t priority)
{
	if (priority < screen_priority[y][x]) {
		screen_priority[y][x] = priority;
		screen_color[y][x] = color;
	}
}

void draw_line(uint32_t x, uint32_t y, uint32_t line,
	       volatile struct palette *palette, uint16_t priority)
{
	if (line == 0 || (x >= GBA_WIDTH && x < UINT32_MAX - GBA_WIDTH)) {
		return;
	}
	for (size_t i = 0; i < 8; ++i) {
		size_t col_index = (line >> (i * 4)) & 0xF;
		assert(col_index < ARRAY_SIZE(palette->color));
		if (col_index == 0) {
			continue;
		}
		uint16_t color = palette->color[col_index];
		draw_pixel(x + i + 8, y, color, priority);
	}
}

void draw_background(enum background bg, uint32_t y, uint16_t priority)
{
	uint32_t scroll_x = *reg_bg_scroll_x(bg);
	uint32_t scroll_y = 0xFFFF - *reg_bg_scroll_y(bg);

	uint32_t bg_y = y - scroll_y;
	uint32_t tile_y = (bg_y / 8) % 32;
	uint32_t tile_line = bg_y % 8;
	uint32_t tile_x_min = scroll_x / 8;

	uint16_t bg_control = *reg_bg_control(bg);

	volatile uint16_t *screen_block =
		screen_block_begin(GET(BGCNT_SCREEN_BLOCK, bg_control));
	volatile struct character_4bpp *char_block =
		character_block_begin(GET(BGCNT_CHAR_BLOCK, bg_control));

	for (size_t tile_x = tile_x_min; tile_x < tile_x_min + 32; ++tile_x) {
		uint32_t wrapped_tile_x = tile_x % 32;
		uint32_t screen_x = tile_x * 8 - scroll_x;
		uint32_t tile_idx = wrapped_tile_x + tile_y * 32;
		uint16_t tile = screen_block[tile_idx];

		if (GET(TILE_VERTICAL_FLIP, tile)) {
			tile_line = 7 - tile_line;
		}

		volatile struct character_4bpp *character =
			&char_block[GET(TILE_CHAR, tile)];

		uint32_t line = character->lines[tile_line];

		if (GET(TILE_HORIZONTAL_FLIP, tile)) {
			line = reverse_nibbles(line);
		}

		volatile struct palette *palette =
			bg_palette(GET(TILE_PALETTE, tile));

		draw_line(screen_x, y, line, palette, priority);
	}
}

void render_entire_line(uint32_t y)
{
	uint16_t bg_color = bg_palette(0)->color[0];
	clear_line(bg_color, y);

	uint16_t display_control = REG_DISPCNT;
	if (GET(DISPCNT_SCREEN_DISPLAY_BG0, display_control)) {
		uint16_t bg_control = *reg_bg_control(BG0);
		uint16_t bg_priority = GET(BGCNT_PRIORITY, bg_control);
		uint16_t priority = (bg_priority << 2) + BG0;
		draw_background(BG0, y, priority);
	}
}

void quit(void)
{
	exit(0);
}

void handle_sdl_event(const SDL_Event *event)
{
	switch (event->type) {
	case SDL_QUIT:
		quit();
		break;
	case SDL_KEYDOWN:
		if (event->key.keysym.sym == SDLK_ESCAPE) {
			quit();
		}
		break;
	}
}

void update_surface_from_screen(void)
{
	uint16_t *pixels = surface->pixels;
	for (size_t y = 0; y < GBA_HEIGHT; ++y) {
		for (size_t x = 8; x < GBA_WIDTH + 8; ++x) {
			uint16_t col = screen_color[y][x];
			uint16_t rotated = GET(COL_RED, col) << 10 |
					   GET(COL_GREEN, col) << 5 |
					   GET(COL_BLUE, col);
			*pixels++ = rotated;
		}
	}
}

void present_frame_and_handle_events(void)
{
	uint32_t now = SDL_GetTicks();
	uint32_t elapsed = now - ticks_previous;
	ticks_previous = now;
	ticks_lag += elapsed;
	frame_counter++;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		handle_sdl_event(&event);
	}

	update_surface_from_screen();
	SDL_UpdateTexture(texture, NULL, surface->pixels, surface->pitch);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

	uint32_t duration_per_frame = 16;
	while (ticks_lag > duration_per_frame) {
		ticks_lag -= duration_per_frame;
	}
	uint32_t time_left = duration_per_frame - ticks_lag;
	if (time_left > 0) {
		SDL_Delay(time_left);
	}
}

void step_emulated_hardware(void)
{
	uint16_t y = REG_VCOUNT;
	uint16_t horizontal_blank =
		GET(DISPSTAT_HORIZONTAL_BLANK, REG_DISPSTAT);

	if (y < GBA_HEIGHT) {
		if (horizontal_blank) {
			REG_VCOUNT = y + 1;
			REG_DISPSTAT &= ~DISPSTAT_HORIZONTAL_BLANK;
		} else {
			REG_DISPSTAT |= DISPSTAT_HORIZONTAL_BLANK;
			render_entire_line(y);
		}
	} else if (y < 227) {
		if (horizontal_blank) {
			REG_VCOUNT = y + 1;
			REG_DISPSTAT &= ~DISPSTAT_HORIZONTAL_BLANK;
			REG_DISPSTAT |= DISPSTAT_VERTICAL_BLANK;
		} else {
			REG_DISPSTAT |= DISPSTAT_HORIZONTAL_BLANK;
		}
	} else {
		if (horizontal_blank) {
			REG_VCOUNT = 0;
			REG_DISPSTAT &= ~DISPSTAT_HORIZONTAL_BLANK;
			present_frame_and_handle_events();
		} else {
			REG_DISPSTAT |= DISPSTAT_HORIZONTAL_BLANK;
		}
	}
}

volatile struct palette *bg_palette(uint32_t palette_idx)
{
	return (volatile struct palette *)(&bg_pallete_ram[palette_idx]);
}

volatile struct character_4bpp *character_block_begin(uint32_t char_block)
{
	return (volatile struct character_4bpp
			*)(&video_ram[char_block * 0x4000]);
}

volatile uint16_t *screen_block_begin(uint32_t screen_block)
{
	return (volatile uint16_t *)(&video_ram[screen_block * 0x800]);
}

volatile uint16_t *reg_bg_control(enum background bg)
{
	return (volatile uint16_t *)&reg_bg_controls[bg];
}

volatile uint16_t *reg_bg_scroll_x(enum background bg)
{
	return (volatile uint16_t *)&reg_bg_scrolls_x[bg];
}

volatile uint16_t *reg_bg_scroll_y(enum background bg)
{
	return (volatile uint16_t *)&reg_bg_scrolls_y[bg];
}

void halt()
{
	assert(REG_IME && "halted without master interrupt flag enabled");
	assert(REG_IE && "halted without interrupts enabled");

	uint32_t steps_left = 0xFFFF;
	while (--steps_left) {
		step_emulated_hardware();

		if (GET(INT_HORIZONTAL_BLANK, REG_IE) &&
		    GET(DISPSTAT_HORIZONTAL_BLANK_IRQ_ENABLED, REG_DISPSTAT) &&
		    GET(DISPSTAT_HORIZONTAL_BLANK, REG_DISPSTAT)) {
			irq_handler();
			return;
		}

		if (GET(INT_VERTICAL_BLANK, REG_IE) &&
		    GET(DISPSTAT_VERTICAL_BLANK_IRQ_ENABLED, REG_DISPSTAT) &&
		    GET(DISPSTAT_VERTICAL_BLANK, REG_DISPSTAT)) {
			irq_handler();
			return;
		}
	}

	assert(steps_left && "halted for a long time, probably a bug?");
}
