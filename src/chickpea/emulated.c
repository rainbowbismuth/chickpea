#include <stdio.h>
#include "chickpea.h"
#include "chickpea/nano_unit.h"
#include "SDL.h"
#include "SDL_timer.h"

#define PRIORITY_LAYER FIELD(0, 6)
#define PRIORITY_UPPER FIELD(6, 3)

uint16_t reg_dispcnt = 0;
uint16_t reg_dispstat = 0;
uint16_t reg_vcount = 0;
uint16_t reg_bldcnt = 0;
uint16_t reg_bldalpha = 0;
uint16_t reg_bldy = 0;
uint16_t reg_keyinput = 0;
uint16_t reg_ie = 0;
uint16_t reg_if = 0;
uint32_t reg_ime = 0;

uint16_t reg_bg_controls[4] = { 0 };
uint16_t reg_bg_scrolls_x[4] = { 0 };
uint16_t reg_bg_scrolls_y[4] = { 0 };

uint8_t video_ram[0x18000] = { 0 };

struct palette bg_pallete_ram[16] = { { 0 } };
struct palette obj_pallete_ram[16] = { { 0 } };
struct object_attribute_mem obj_attr_mem = { 0 };

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Surface *surface = NULL;
SDL_Texture *texture = NULL;
SDL_GameController *nullable controller = NULL;

uint32_t ticks_previous = 0;
uint32_t ticks_lag = 0;
uint64_t frame_counter = 0;

uint32_t real_win_width = GBA_WIDTH * 2;
uint32_t real_win_height = GBA_HEIGHT * 2;

uint16_t screen_color[GBA_HEIGHT][GBA_WIDTH] = { 0 };
uint16_t screen_priority[GBA_HEIGHT][GBA_WIDTH] = { 0 };

extern struct nano_unit_suite test_suites[];

bool flag_present(int argc, const char *nonnull argv[],
		  const char *nonnull flag)
{
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], flag) == 0) {
			return true;
		}
	}
	return false;
}

int main(int argc, const char *nonnull argv[])
{
	bool any_failures = nano_unit_run_suites(test_suites);
	if (flag_present(argc, argv, "--tests-only")) {
		return any_failures;
	}

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	atexit(SDL_Quit);
	window = SDL_CreateWindow("chickpea-c", SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED, real_win_width,
				  real_win_height, SDL_WINDOW_ALLOW_HIGHDPI);
	assert(window != NULL);

	renderer = SDL_CreateRenderer(window, -1,
				      SDL_RENDERER_ACCELERATED |
					      SDL_RENDERER_PRESENTVSYNC);
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

static void clear_line(uint16_t bg_color, uint16_t y)
{
	for (size_t i = 0; i < ARRAY_SIZE(screen_color[0]); ++i) {
		screen_color[y][i] = bg_color;
		screen_priority[y][i] = PREP(PRIORITY_UPPER, 0x3) |
					PREP(PRIORITY_LAYER, 1 << 5);
	}
}

static void draw_pixel(uint32_t x, uint32_t y, uint16_t color,
		       uint16_t priority)
{
	if (priority < screen_priority[y][x]) {
		screen_priority[y][x] = priority;
		screen_color[y][x] = color;
	}
}

struct blend_alpha_params {
	uint16_t target_mask;
	uint16_t src_weight;
	uint16_t dst_weight;
};

static void
draw_pixel_blend_alpha(uint32_t x, uint32_t y, uint16_t color,
		       uint16_t priority,
		       const struct blend_alpha_params *nonnull params)
{
	uint16_t target_priority = screen_priority[y][x];
	if (priority < target_priority) {
		screen_priority[y][x] = priority;
		uint32_t target_layer = GET(PRIORITY_LAYER, target_priority);
		if ((target_layer & params->target_mask) != 0) {
			screen_color[y][x] = additive_blend(color,
							    params->src_weight,
							    screen_color[y][x],
							    params->dst_weight);
		} else {
			screen_color[y][x] = color;
		}
	}
}

static void draw_line(uint32_t x, uint32_t y, uint32_t line,
		      const struct palette *nonnull palette, uint16_t priority)
{
	if (line == 0) {
		return;
	}
	uint16_t blend_control = REG_BLDCNT;

	if (GET(BLDCNT_EFFECT, blend_control) == BLEND_ALPHA &&
	    (GET(PRIORITY_LAYER, priority) &
	     GET(BLDCNT_1ST_TARGET, blend_control)) != 0) {
		struct blend_alpha_params params = {
			.target_mask = GET(BLDCNT_2ND_TARGET, blend_control),
			.src_weight = GET(BLDALPHA_1ST_WEIGHT, REG_BLDALPHA),
			.dst_weight = GET(BLDALPHA_2ND_WEIGHT, REG_BLDALPHA)
		};

		for (size_t i = 0; i < 8; ++i) {
			size_t col_index = (line >> (i * 4)) & 0xF;
			if (col_index == 0) {
				continue;
			}
			uint16_t color = palette->color[col_index];
			uint32_t new_x = (x + i) & 0x1FF;
			if (new_x >= GBA_WIDTH) {
				continue;
			}
			draw_pixel_blend_alpha(new_x, y, color, priority,
					       &params);
		}
	} else {
		assert(GET(BLDCNT_EFFECT, blend_control) !=
			       BLEND_BRIGHTNESS_DECREASE &&
		       "effect not implemented");
		assert(GET(BLDCNT_EFFECT, blend_control) !=
			       BLEND_BRIGHTNESS_INCREASE &&
		       "effect not implemented");

		for (size_t i = 0; i < 8; ++i) {
			size_t col_index = (line >> (i * 4)) & 0xF;
			if (col_index == 0) {
				continue;
			}
			uint16_t color = palette->color[col_index];
			uint32_t new_x = (x + i) & 0x1FF;
			if (new_x >= GBA_WIDTH) {
				continue;
			}
			draw_pixel(new_x, y, color, priority);
		}
	}
};

static void draw_background(enum background bg, uint32_t y, uint16_t priority)
{
	uint32_t scroll_x = reg_bg_scrolls_x[bg];
	uint32_t scroll_y = 0xFFFF - reg_bg_scrolls_y[bg];

	uint32_t bg_y = y - scroll_y;
	uint32_t tile_y = (bg_y / 8) % 32;
	uint32_t tile_line = bg_y % 8;
	uint32_t tile_x_min = scroll_x / 8;

	uint16_t bg_control = *reg_bg_control(bg);

	const uint16_t *screen_block = (uint16_t *)screen_block_begin(
		GET(BGCNT_SCREEN_BLOCK, bg_control));

	const struct char_4bpp *char_block =
		(struct char_4bpp *)character_block_begin(
			GET(BGCNT_CHAR_BLOCK, bg_control));

	for (size_t tile_x = tile_x_min; tile_x < tile_x_min + 32; ++tile_x) {
		uint32_t wrapped_tile_x = tile_x % 32;
		uint32_t screen_x = tile_x * 8 - scroll_x;
		uint32_t tile_idx = wrapped_tile_x + tile_y * 32;
		uint16_t tile = screen_block[tile_idx];

		uint32_t line_idx = tile_line;
		if (GET(TILE_VERTICAL_FLIP, tile)) {
			line_idx = 7 - tile_line;
		}

		struct char_4bpp *character =
			(struct char_4bpp *)&char_block[GET(TILE_CHAR, tile)];

		uint32_t line = character->lines[line_idx];

		if (GET(TILE_HORIZONTAL_FLIP, tile)) {
			line = reverse_nibbles(line);
		}

		struct palette *palette =
			(struct palette *)bg_palette(GET(TILE_PALETTE, tile));

		draw_line(screen_x, y, line, palette, priority);
	}
}

struct background_array {
	enum background bgs[4];
};

static void sort_backgrounds_by_priority(struct background_array *arr)
{
	/*
	 * Standard insertion sort. Higher priority values first.
	 */
	size_t i = 1;
	while (i < ARRAY_SIZE(arr->bgs)) {
		size_t j = i;
		uint16_t j_m1_priority =
			GET(BGCNT_PRIORITY, *reg_bg_control(arr->bgs[j - 1]));
		uint16_t j_priority =
			GET(BGCNT_PRIORITY, *reg_bg_control(arr->bgs[j]));
		while (j > 0 && j_priority > j_m1_priority) {
			enum background tmp = arr->bgs[j - 1];
			arr->bgs[j - 1] = arr->bgs[j];
			arr->bgs[j] = tmp;
			j--;
		}
		i++;
	}
}

static void draw_obj_char(uint32_t y, struct char_4bpp *nonnull character,
			  struct palette *nonnull pal, uint16_t priority,
			  uint32_t start_x, uint32_t start_y,
			  bool vertical_flip, bool horizontal_flip)
{
	// TODO: Improve this very silly loop
	bool ok = false;
	for (size_t i = 0; i < 8; ++i) {
		ok = ok || y == ((start_y + i) & 0xFF);
	}
	if (!ok) {
		return;
	}

	uint8_t line_y = y - start_y;
	if (vertical_flip) {
		line_y = 7 - line_y;
	}

	uint32_t line = character->lines[line_y];
	if (horizontal_flip) {
		line = reverse_nibbles(line);
	}

	draw_line(start_x, y, line, pal, priority);
}

static void draw_object(uint32_t y, const struct oam_entry *nonnull obj)
{
	if (GET(OBJA0_OBJ_DISABLE, obj->attr_0)) {
		return;
	}

	uint32_t start_y = GET(OBJA0_Y, obj->attr_0);
	uint32_t start_x = GET(OBJA1_X, obj->attr_1);

	// Only handling mode 0 right now.
	struct char_4bpp *char_block =
		(struct char_4bpp *)character_block_begin(4);

	uint32_t char_name = GET(OBJA2_CHAR, obj->attr_2);
	struct palette *palette =
		(struct palette *)obj_palette(GET(OBJA2_PALETTE, obj->attr_2));

	uint16_t priority =
		PREP(PRIORITY_UPPER, GET(OBJA2_PRIORITY, obj->attr_2));

	enum obj_shape shape = GET(OBJA0_SHAPE, obj->attr_0);
	enum obj_size size = GET(OBJA1_SIZE, obj->attr_1);
	bool vertical_flip = GET(OBJA1_VERTICAL_FLIP, obj->attr_1);
	bool horizontal_flip = GET(OBJA1_HORIZONTAL_FLIP, obj->attr_1);

	size_t width = object_width(shape, size);
	size_t height = object_height(shape, size);

	// TODO: It would be smarter to check if we are within the right Y
	//   and not do this silly looping business

	// TODO: The flipping
	for (size_t i = 0; i < width; ++i) {
		for (size_t j = 0; j < height; ++j) {
			uint32_t obj_x = (start_x + i * 8) & 0x1FF;
			uint32_t obj_y = (start_y + j * 8) & 0xFF;
			draw_obj_char(y, &char_block[char_name], palette,
				      priority, obj_x, obj_y, vertical_flip,
				      horizontal_flip);

			char_name++;
		}
	}
}

static void render_entire_line(uint32_t y)
{
	uint16_t bg_color = bg_palette(0)->color[0];
	clear_line(bg_color, y);

	struct background_array arr = { .bgs = { BG0, BG1, BG2, BG3 } };
	sort_backgrounds_by_priority(&arr);

	uint16_t display_control = REG_DISPCNT;

	for (size_t i = 0; i < ARRAY_SIZE(arr.bgs); ++i) {
		enum background bg = arr.bgs[i];
		if ((GET(DISPCNT_SCREEN_BG_ENABLED, display_control) &
		     (1 << bg)) == 0) {
			continue;
		}
		uint16_t bg_control = *reg_bg_control(bg);
		uint16_t bg_priority = GET(BGCNT_PRIORITY, bg_control);
		uint16_t priority = PREP(PRIORITY_UPPER, bg_priority) |
				    (1 << bg);
		draw_background(bg, y, priority);
	}

	if (!GET(DISPCNT_SCREEN_DISPLAY_OBJ, display_control)) {
		return;
	}

	for (size_t i = 0; i < ARRAY_SIZE(obj_attr_mem.entries); ++i) {
		draw_object(y, &obj_attr_mem.entries[i]);
	}
}

static void quit(void)
{
	exit(0);
}

static void handle_sdl_event(const SDL_Event *nonnull event)
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

static void update_surface_from_screen(void)
{
	uint16_t *pixels = surface->pixels;
	for (size_t y = 0; y < GBA_HEIGHT; ++y) {
		for (size_t x = 0; x < GBA_WIDTH; ++x) {
			uint16_t col = screen_color[y][x];
			uint16_t rotated = GET(COL_RED, col) << 10 |
					   GET(COL_GREEN, col) << 5 |
					   GET(COL_BLUE, col);
			*pixels++ = rotated;
		}
	}
}

static void find_game_controller_if_none(void)
{
	if (controller) {
		if (SDL_GameControllerGetAttached(controller)) {
			return;
		} else {
			SDL_GameControllerClose(controller);
			controller = NULL;
		}
	}

	for (int i = 0; i < SDL_NumJoysticks(); ++i) {
		if (SDL_IsGameController(i)) {
			controller = SDL_GameControllerOpen(i);
			if (controller) {
				break;
			} else {
				fprintf(stderr,
					"Couldn't open controller %i: %s\n", i,
					SDL_GetError());
			}
		}
	}
}

static void update_game_controller(void)
{
	find_game_controller_if_none();
	if (!controller) {
		REG_KEYINPUT = ~0;
		return;
	}
	/*
	 * The SDL buttons are as on X-BOX like controller, so the A/B X/Y are
	 * flipped here to match a SNES like controller.
	 */
	uint16_t input = 0;
	input |= PREP(KEYINPUT_BUTTON_A,
		      SDL_GameControllerGetButton(controller,
						  SDL_CONTROLLER_BUTTON_B));
	input |= PREP(KEYINPUT_BUTTON_B,
		      SDL_GameControllerGetButton(controller,
						  SDL_CONTROLLER_BUTTON_A));
	input |= PREP(KEYINPUT_SELECT,
		      SDL_GameControllerGetButton(controller,
						  SDL_CONTROLLER_BUTTON_BACK));
	input |= PREP(KEYINPUT_START,
		      SDL_GameControllerGetButton(controller,
						  SDL_CONTROLLER_BUTTON_START));
	input |= PREP(KEYINPUT_RIGHT,
		      SDL_GameControllerGetButton(
			      controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT));
	input |= PREP(KEYINPUT_LEFT,
		      SDL_GameControllerGetButton(
			      controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT));
	input |= PREP(KEYINPUT_UP,
		      SDL_GameControllerGetButton(
			      controller, SDL_CONTROLLER_BUTTON_DPAD_UP));
	input |= PREP(KEYINPUT_DOWN,
		      SDL_GameControllerGetButton(
			      controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN));
	input |= PREP(KEYINPUT_BUTTON_L,
		      SDL_GameControllerGetButton(
			      controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER));
	input |= PREP(KEYINPUT_BUTTON_R,
		      SDL_GameControllerGetButton(
			      controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER));
	input |= PREP(KEYINPUT_BUTTON_X,
		      SDL_GameControllerGetButton(controller,
						  SDL_CONTROLLER_BUTTON_Y));
	input |= PREP(KEYINPUT_BUTTON_Y,
		      SDL_GameControllerGetButton(controller,
						  SDL_CONTROLLER_BUTTON_X));
	REG_KEYINPUT = ~input;
}

static void present_frame_and_handle_events(void)
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
	update_game_controller();

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

static void step_emulated_hardware(void)
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

volatile struct palette *nonnull bg_palette(uint32_t palette_idx)
{
	assert(palette_idx < ARRAY_SIZE(bg_pallete_ram));
	return (volatile struct palette *)(&bg_pallete_ram[palette_idx]);
}

volatile struct palette *nonnull obj_palette(uint32_t palette_idx)
{
	assert(palette_idx < ARRAY_SIZE(obj_pallete_ram));
	return (volatile struct palette *)(&obj_pallete_ram[palette_idx]);
}

volatile struct char_4bpp *nonnull character_block_begin(uint32_t char_block)
{
	assert(char_block < 5);
	return (volatile struct char_4bpp *)(&video_ram[char_block * 0x4000]);
}

volatile uint16_t *nonnull screen_block_begin(uint32_t screen_block)
{
	assert(screen_block < 32);
	return (volatile uint16_t *)(&video_ram[screen_block * 0x800]);
}

volatile uint16_t *nonnull reg_bg_control(enum background bg)
{
	return (volatile uint16_t *)&reg_bg_controls[bg];
}

void set_bg_scroll_x(enum background bg, uint16_t scroll_x)
{
	reg_bg_scrolls_x[bg] = scroll_x;
}

void set_bg_scroll_y(enum background bg, uint16_t scroll_y)
{
	reg_bg_scrolls_y[bg] = scroll_y;
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

void cpu_fast_set(const void *restrict nonnull src, void *restrict nonnull dst,
		  size_t word_count)
{
	assert(((size_t)src & 0x3) == 0 && "must be aligned by 4");
	assert(((size_t)dst & 0x3) == 0 && "must be aligned by 4");
	assert(word_count % 8 == 0 && "must be multiple of 8 words");
	memcpy(dst, src, word_count * 4);
}

void cpu_fast_fill(uint32_t src, void *nonnull dst, size_t word_count)
{
	assert(((size_t)dst & 0x3) == 0 && "must be aligned by 4");
	assert(word_count % 8 == 0 && "must be multiple of 8 words");
	for (size_t i = 0; i < word_count; ++i) {
		((uint32_t *)dst)[i] = src;
	}
}

void debug_put_char(char c)
{
	fputc(c, stdout);
}

void debug_put_str(const char *nonnull str)
{
	fputs(str, stdout);
}