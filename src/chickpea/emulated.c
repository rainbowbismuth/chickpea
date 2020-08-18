#include "SDL.h"
#include "SDL_timer.h"
#include "chickpea.h"
#include "chickpea/nano_unit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif

#define PRIORITY_2ND_TARGET BIT(0)
#define PRIORITY_1ST_TARGET BIT(1)
#define PRIORITY_LAYER	    FIELD(3, 6)
#define PRIORITY_UPPER	    FIELD(9, 3)

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
const uint8_t *keyboard_state = NULL;

uint32_t ticks_previous = 0;
uint32_t ticks_lag = 0;
uint64_t frame_counter = 0;

uint32_t real_win_width = GBA_WIDTH * 3;
uint32_t real_win_height = GBA_HEIGHT * 3;

uint16_t screen_color[GBA_HEIGHT][GBA_WIDTH] = { 0 };
uint16_t screen_priority[GBA_WIDTH] = { 0 };

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

#ifdef EMSCRIPTEN
static void emscripten_game_loop(void)
{
	while (!game_update()) {
		;
	}
}
#endif

int main(int argc, const char *nonnull argv[])
{
	bool any_failures = nano_unit_run_suites(test_suites);
	if (flag_present(argc, argv, "--tests-only")) {
		return any_failures;
	}

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
	atexit(SDL_Quit);
	window = SDL_CreateWindow("Chickpea", SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED, real_win_width,
				  real_win_height, SDL_WINDOW_ALLOW_HIGHDPI);
	assert(window);

	renderer = SDL_CreateRenderer(window, -1,
				      SDL_RENDERER_ACCELERATED
					      | SDL_RENDERER_PRESENTVSYNC);
	assert(renderer);

	surface = SDL_CreateRGBSurfaceWithFormat(0, GBA_WIDTH, GBA_HEIGHT, 16,
						 SDL_PIXELFORMAT_RGB555);
	assert(surface);

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB555,
				    SDL_TEXTUREACCESS_STREAMING, GBA_WIDTH,
				    GBA_HEIGHT);
	assert(texture);

	keyboard_state = SDL_GetKeyboardState(NULL);
	assert(keyboard_state);

	game_init();

#ifdef EMSCRIPTEN
	emscripten_set_main_loop(emscripten_game_loop, -1, 1);
#else
	while (1) {
		game_update();
	}
#endif
}

static void clear_line(uint16_t bg_color, uint16_t y)
{
	uint16_t blend_target_bits = 0;
	if (GET(BLDCNT_1ST_TARGET_BD, REG_BLDCNT)) {
		blend_target_bits |= PRIORITY_1ST_TARGET;
	}
	if (GET(BLDCNT_2ND_TARGET, REG_BLDCNT)) {
		blend_target_bits |= PRIORITY_2ND_TARGET;
	}
	for (size_t i = 0; i < ARRAY_SIZE(screen_color[0]); ++i) {
		screen_color[y][i] = bg_color;
		screen_priority[i] = PREP(PRIORITY_UPPER, 0x3)
				   | PREP(PRIORITY_LAYER, 1 << 5)
				   | blend_target_bits;
	}
}

static void draw_pixel(uint32_t x, uint32_t y, uint16_t color,
		       uint16_t priority)
{
	uint16_t current = screen_priority[x];
	if (priority >= current) {
		color = screen_color[y][x];
		screen_priority[x] =
			current & ~(PRIORITY_1ST_TARGET | PRIORITY_2ND_TARGET);
	} else {
		screen_priority[x] = priority;
	}
	screen_color[y][x] = color;
}

struct blend_alpha_params {
	uint16_t src_weight;
	uint16_t dst_weight;
};

static void
draw_pixel_blend_alpha(uint32_t x, uint32_t y, uint16_t color,
		       uint16_t priority,
		       const struct blend_alpha_params *nonnull params)
{
	uint16_t current = screen_priority[x];
	if (priority >= current) {
		if (current & PRIORITY_1ST_TARGET
		    && priority & PRIORITY_2ND_TARGET) {
			color = additive_blend(screen_color[y][x],
					       params->src_weight, color,
					       params->dst_weight);
			screen_priority[x] =
				priority
				& ~(PRIORITY_1ST_TARGET | PRIORITY_2ND_TARGET);
		} else {
			color = screen_color[y][x];
			screen_priority[x] =
				current
				& ~(PRIORITY_1ST_TARGET | PRIORITY_2ND_TARGET);
		}
	} else {
		screen_priority[x] = priority & ~PRIORITY_2ND_TARGET;
	}
	screen_color[y][x] = color;
}

static void draw_line(uint32_t x, uint32_t y, uint32_t line,
		      const struct palette *nonnull palette, uint16_t priority)
{
	if (line == 0) {
		return;
	}
	uint16_t blend_control = REG_BLDCNT;

	if (GET(BLDCNT_EFFECT, blend_control) == BLEND_ALPHA
	    && GET(PRIORITY_2ND_TARGET, priority)) {
		struct blend_alpha_params params = {
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
		assert(GET(BLDCNT_EFFECT, blend_control)
			       != BLEND_BRIGHTNESS_DECREASE
		       && "effect not implemented");
		assert(GET(BLDCNT_EFFECT, blend_control)
			       != BLEND_BRIGHTNESS_INCREASE
		       && "effect not implemented");

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
}

static void draw_line_256(uint32_t x, uint32_t y, uint64_t line,
			  const uint16_t *nonnull palette, uint16_t priority)
{
	if (line == 0) {
		return;
	}
	uint16_t blend_control = REG_BLDCNT;

	if (GET(BLDCNT_EFFECT, blend_control) == BLEND_ALPHA
	    && GET(PRIORITY_2ND_TARGET, priority)) {
		struct blend_alpha_params params = {
			.src_weight = GET(BLDALPHA_1ST_WEIGHT, REG_BLDALPHA),
			.dst_weight = GET(BLDALPHA_2ND_WEIGHT, REG_BLDALPHA)
		};

		for (size_t i = 0; i < 8; ++i) {
			size_t col_index = (line >> (i * 8)) & 0xFF;
			if (col_index == 0) {
				continue;
			}
			uint16_t color = palette[col_index];
			uint32_t new_x = (x + i) & 0x1FF;
			if (new_x >= GBA_WIDTH) {
				continue;
			}
			draw_pixel_blend_alpha(new_x, y, color, priority,
					       &params);
		}
	} else {
		assert(GET(BLDCNT_EFFECT, blend_control)
			       != BLEND_BRIGHTNESS_DECREASE
		       && "effect not implemented");
		assert(GET(BLDCNT_EFFECT, blend_control)
			       != BLEND_BRIGHTNESS_INCREASE
		       && "effect not implemented");

		for (size_t i = 0; i < 8; ++i) {
			size_t col_index = (line >> (i * 8)) & 0xFF;
			if (col_index == 0) {
				continue;
			}
			uint16_t color = palette[col_index];
			uint32_t new_x = (x + i) & 0x1FF;
			if (new_x >= GBA_WIDTH) {
				continue;
			}
			draw_pixel(new_x, y, color, priority);
		}
	}
}

static uint32_t bg_width_tiles(uint16_t bg_control)
{
	switch (GET(BGCNT_SCREEN_SIZE, bg_control)) {
	case 1:
	case 3:
		return 64;
	default:
		assert(false && "not possible");
	case 0:
	case 2:
		return 32;
	}
}

static uint32_t bg_height_tiles(uint16_t bg_control)
{
	switch (GET(BGCNT_SCREEN_SIZE, bg_control)) {
	case 2:
	case 3:
		return 64;
	default:
		assert(false && "not possible");
	case 0:
	case 1:
		return 32;
	}
}

static void draw_background(enum background bg, uint32_t y, uint16_t priority)
{
	if ((GET(DISPCNT_SCREEN_BG_ENABLED, REG_DISPCNT) & (1 << bg)) == 0) {
		return;
	}

	uint32_t scroll_x = reg_bg_scrolls_x[bg];
	uint32_t scroll_y = 0xFFFF - reg_bg_scrolls_y[bg];

	uint16_t bg_control = *reg_bg_control(bg);

	uint32_t bg_width = bg_width_tiles(bg_control);
	uint32_t bg_height = bg_height_tiles(bg_control);

	uint32_t bg_y = y - scroll_y;
	uint32_t tile_y = (bg_y / 8) % bg_height;
	uint32_t tile_line = bg_y % 8;
	uint32_t tile_x_min = scroll_x / 8;

	const uint16_t *screen_block = (uint16_t *)screen_block_begin(
		GET(BGCNT_SCREEN_BLOCK, bg_control));

	if (tile_y >= 32) {
		screen_block += bg_width * 32;
	}

	const struct char_4bpp *char_block =
		(struct char_4bpp *)char_block_begin(
			GET(BGCNT_CHAR_BLOCK, bg_control));

	for (size_t tile_x = tile_x_min; tile_x < tile_x_min + 32; ++tile_x) {
		uint32_t wrapped_tile_x = tile_x % bg_width;
		uint32_t screen_x = tile_x * 8 - scroll_x;
		uint32_t tile_idx = (wrapped_tile_x % 32) + tile_y * 32;

		if (wrapped_tile_x >= 32) {
			tile_idx += 32 * 32;
		}

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
	 * Standard insertion sort. Lower priority backgrounds come first
	 * so that rendering is top-most pixel to back-most pixel.
	 */
	size_t i = 1;
	while (i < ARRAY_SIZE(arr->bgs)) {
		size_t j = i;

		/* (╯°□°)╯︵ ┻━┻ sorry for huge conditional */
		while (j > 0
		       && (PREP(PRIORITY_UPPER,
				GET(BGCNT_PRIORITY,
				    *reg_bg_control(arr->bgs[j - 1])))
			   | PREP(PRIORITY_LAYER, arr->bgs[j - 1]))
				  > (PREP(PRIORITY_UPPER,
					  GET(BGCNT_PRIORITY,
					      *reg_bg_control(arr->bgs[j])))
				     | PREP(PRIORITY_LAYER, arr->bgs[j]))) {
			enum background tmp = arr->bgs[j - 1];
			arr->bgs[j - 1] = arr->bgs[j];
			arr->bgs[j] = tmp;
			j--;
		}
		i++;
	}
}

static uint64_t reverse_bytes(uint64_t n)
{
	n = (n & 0x00FF00FF00FF00FF) << 8 | (n & 0xFF00FF00FF00FF00) >> 8;
	n = (n & 0x0000FFFF0000FFFF) << 16 | (n & 0xFFFF0000FFFF0000) >> 16;
	n = (n & 0x00000000FFFFFFFF) << 32 | (n & 0xFFFFFFFF00000000) >> 32;
	return n;
}

static void draw_obj_char(uint32_t y, struct char_4bpp *nonnull character,
			  struct palette *nonnull pal, uint16_t priority,
			  uint32_t start_x, uint32_t start_y,
			  bool vertical_flip, bool horizontal_flip,
			  bool colors_256)
{
	if (start_y + 8 <= 0xFF) {
		if (y < start_y || y >= start_y + 8) {
			return;
		}
	} else {
		if (y >= ((start_y + 8) & 0xFF) && y < start_y) {
			return;
		}
	}

	uint8_t line_y = y - start_y;
	if (vertical_flip) {
		line_y = 7 - line_y;
	}

	if (colors_256) {
		uint64_t line = ((struct char_8bpp *)character)->lines[line_y];
		if (horizontal_flip) {
			line = reverse_bytes(line);
		}
		draw_line_256(start_x, y, line, (uint16_t *)obj_pallete_ram,
			      priority);
	} else {
		uint32_t line = character->lines[line_y];
		if (horizontal_flip) {
			line = reverse_nibbles(line);
		}
		draw_line(start_x, y, line, pal, priority);
	}
}

static void draw_object(uint32_t y, const struct oam_entry *nonnull obj)
{
	if (GET(OBJA0_OBJ_DISABLE, obj->attr_0)) {
		return;
	}

	// TODO: Still not sure why everything is off by 1 pix?
	uint16_t start_y = GET(OBJA0_Y, obj->attr_0) - 1;
	uint16_t start_x = GET(OBJA1_X, obj->attr_1);

	// Only handling mode 0 right now.
	struct char_4bpp *char_block = (struct char_4bpp *)char_block_begin(4);

	uint32_t char_name = GET(OBJA2_CHAR, obj->attr_2);
	struct palette *palette =
		(struct palette *)obj_palette(GET(OBJA2_PALETTE, obj->attr_2));

	uint16_t priority =
		PREP(PRIORITY_UPPER, GET(OBJA2_PRIORITY, obj->attr_2));

	if (GET(BLDCNT_1ST_TARGET_OBJ, REG_BLDCNT)) {
		priority |= PRIORITY_1ST_TARGET;
	}
	if (GET(BLDCNT_2ND_TARGET_OBJ, REG_BLDCNT)) {
		priority |= PRIORITY_2ND_TARGET;
	}

	enum obj_shape shape = GET(OBJA0_SHAPE, obj->attr_0);
	enum obj_size size = GET(OBJA1_SIZE, obj->attr_1);
	bool vertical_flip = GET(OBJA1_VERTICAL_FLIP, obj->attr_1);
	bool horizontal_flip = GET(OBJA1_HORIZONTAL_FLIP, obj->attr_1);
	bool colors_256 = GET(OBJA0_256_COLORS, obj->attr_0);

	size_t width = object_width(shape, size);
	size_t height = object_height(shape, size);

	// TODO: It would be smarter to check if we are within the right Y
	//   and not do this silly looping business

	// TODO: The flipping
	for (size_t i = 0; i < height; ++i) {
		for (size_t j = 0; j < width; ++j) {
			uint32_t obj_x = (start_x + j * 8) & 0x1FF;
			uint32_t obj_y = (start_y + i * 8) & 0xFF;
			draw_obj_char(y, &char_block[char_name], palette,
				      priority, obj_x, obj_y, vertical_flip,
				      horizontal_flip, colors_256);

			char_name += colors_256 ? 2 : 1;
		}
	}
}

static void draw_objects_with_bg_priority(uint32_t y, uint8_t bg_priority)
{
	if (!GET(DISPCNT_SCREEN_DISPLAY_OBJ, REG_DISPCNT)) {
		return;
	}
	for (size_t i = 0; i < ARRAY_SIZE(obj_attr_mem.entries); ++i) {
		struct oam_entry *obj = &obj_attr_mem.entries[i];
		if (GET(OBJA2_PRIORITY, obj->attr_2) == bg_priority) {
			draw_object(y, obj);
		}
	}
}

static void render_entire_line(uint32_t y)
{
	uint16_t bg_color = bg_palette(0)->color[0];
	clear_line(bg_color, y);

	struct background_array arr = { .bgs = { BG0, BG1, BG2, BG3 } };
	sort_backgrounds_by_priority(&arr);

	uint8_t object_bg_priority = 0;
	draw_objects_with_bg_priority(y, object_bg_priority);

	for (size_t i = 0; i < ARRAY_SIZE(arr.bgs); ++i) {
		enum background bg = arr.bgs[i];

		uint16_t bg_control = *reg_bg_control(bg);
		uint16_t bg_priority = GET(BGCNT_PRIORITY, bg_control);
		uint16_t priority = PREP(PRIORITY_UPPER, bg_priority)
				  | (1 << bg);

		if (GET(BLDCNT_1ST_TARGET, REG_BLDCNT) & (1 << bg)) {
			priority |= PRIORITY_1ST_TARGET;
		}
		if (GET(BLDCNT_2ND_TARGET, REG_BLDCNT) & (1 << bg)) {
			priority |= PRIORITY_2ND_TARGET;
		}

		while (object_bg_priority < bg_priority) {
			object_bg_priority++;
			draw_objects_with_bg_priority(y, object_bg_priority);
		}

		draw_background(bg, y, priority);
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
			uint16_t rotated = GET(COL_RED, col) << 10
					 | GET(COL_GREEN, col) << 5
					 | GET(COL_BLUE, col);
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

static size_t SCANCODE_BUTTON_A = SDL_SCANCODE_C;
static size_t SCANCODE_BUTTON_B = SDL_SCANCODE_X;
static size_t SCANCODE_SELECT = SDL_SCANCODE_S;
static size_t SCANCODE_START = SDL_SCANCODE_D;
static size_t SCANCODE_RIGHT = SDL_SCANCODE_RIGHT;
static size_t SCANCODE_LEFT = SDL_SCANCODE_LEFT;
static size_t SCANCODE_UP = SDL_SCANCODE_UP;
static size_t SCANCODE_DOWN = SDL_SCANCODE_DOWN;
static size_t SCANCODE_BUTTON_L = SDL_SCANCODE_A;
static size_t SCANCODE_BUTTON_R = SDL_SCANCODE_F;
static size_t SCANCODE_BUTTON_X = SDL_SCANCODE_W;
static size_t SCANCODE_BUTTON_Y = SDL_SCANCODE_E;

static uint16_t read_keyboard(void)
{
	uint16_t input = 0;
	input |= PREP(KEYINPUT_BUTTON_A, keyboard_state[SCANCODE_BUTTON_A]);
	input |= PREP(KEYINPUT_BUTTON_B, keyboard_state[SCANCODE_BUTTON_B]);
	input |= PREP(KEYINPUT_SELECT, keyboard_state[SCANCODE_SELECT]);
	input |= PREP(KEYINPUT_START, keyboard_state[SCANCODE_START]);
	input |= PREP(KEYINPUT_RIGHT, keyboard_state[SCANCODE_RIGHT]);
	input |= PREP(KEYINPUT_LEFT, keyboard_state[SCANCODE_LEFT]);
	input |= PREP(KEYINPUT_UP, keyboard_state[SCANCODE_UP]);
	input |= PREP(KEYINPUT_DOWN, keyboard_state[SCANCODE_DOWN]);
	input |= PREP(KEYINPUT_BUTTON_L, keyboard_state[SCANCODE_BUTTON_L]);
	input |= PREP(KEYINPUT_BUTTON_R, keyboard_state[SCANCODE_BUTTON_R]);
	input |= PREP(KEYINPUT_BUTTON_X, keyboard_state[SCANCODE_BUTTON_X]);
	input |= PREP(KEYINPUT_BUTTON_Y, keyboard_state[SCANCODE_BUTTON_Y]);
	return ~input;
}

static uint16_t read_game_controller(void)
{
	find_game_controller_if_none();
	if (!controller) {
		return ~0;
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
	return ~input;
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

	REG_KEYINPUT = read_game_controller() & read_keyboard();

	update_surface_from_screen();
	SDL_UpdateTexture(texture, NULL, surface->pixels, surface->pitch);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

	uint32_t duration_per_frame = 16;
	while (ticks_lag > duration_per_frame) {
		ticks_lag -= duration_per_frame;
	}
#ifndef EMSCRIPTEN
	uint32_t time_left = duration_per_frame - ticks_lag;
	if (time_left > 0) {
		SDL_Delay(time_left);
	}
#endif
}

static bool trigger_vertical_blank = false;

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
		REG_DISPSTAT |= DISPSTAT_VERTICAL_BLANK;
		if (horizontal_blank) {
			REG_VCOUNT = y + 1;
			REG_DISPSTAT &= ~DISPSTAT_HORIZONTAL_BLANK;

		} else {
			REG_DISPSTAT |= DISPSTAT_HORIZONTAL_BLANK;
		}
	} else {
		REG_DISPSTAT &= ~DISPSTAT_VERTICAL_BLANK;
		if (horizontal_blank) {
			REG_DISPSTAT &= ~DISPSTAT_HORIZONTAL_BLANK;
			REG_VCOUNT = 0;
			trigger_vertical_blank = true;
			present_frame_and_handle_events();
		} else {
			REG_DISPSTAT |= DISPSTAT_HORIZONTAL_BLANK;
		}
	}
}

struct palette *nonnull bg_palette(uint32_t palette_idx)
{
	assert(palette_idx < ARRAY_SIZE(bg_pallete_ram));
	return &bg_pallete_ram[palette_idx];
}

struct palette *nonnull obj_palette(uint32_t palette_idx)
{
	assert(palette_idx < ARRAY_SIZE(obj_pallete_ram));
	return &obj_pallete_ram[palette_idx];
}

struct char_4bpp *nonnull char_block_begin(uint32_t char_block)
{
	assert(char_block < 5);
	return (struct char_4bpp *)&video_ram[char_block * 0x4000];
}

uint16_t *nonnull screen_block_begin(uint32_t screen_block)
{
	assert(screen_block < 32);
	return (uint16_t *)(&video_ram[screen_block * 0x800]);
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

		if (GET(INT_HORIZONTAL_BLANK, REG_IE)
		    && GET(DISPSTAT_HORIZONTAL_BLANK_IRQ_ENABLED, REG_DISPSTAT)
		    && GET(DISPSTAT_HORIZONTAL_BLANK, REG_DISPSTAT)) {
			REG_IF |= INT_HORIZONTAL_BLANK;
		}

		if (GET(INT_VERTICAL_BLANK, REG_IE)
		    && GET(DISPSTAT_VERTICAL_BLANK_IRQ_ENABLED, REG_DISPSTAT)
		    && GET(DISPSTAT_VERTICAL_BLANK, REG_DISPSTAT)
		    && trigger_vertical_blank) {
			REG_IF |= INT_VERTICAL_BLANK;
			trigger_vertical_blank = false;
		}

		bool done = REG_IF != 0;
		while (REG_IF && --steps_left) {
			irq_handler();
		}
		assert(steps_left
		       && "irq_handler() loop, not acknowledging interrupt?");
		if (done) {
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

static void gba_lz77_decompress(const uint8_t *restrict nonnull src,
				uint8_t *restrict nonnull dst)
{
	uint32_t remaining = src[3] << 16 | src[2] << 8 | src[1];
	src += 4;
	uint8_t block_header = 0;
	uint8_t blocks_remaining = 0;
	const uint8_t *disp = 0;
	uint8_t bytes = 0;
	while (remaining > 0) {
		if (blocks_remaining) {
			if (block_header & 0x80) {
				uint16_t block = src[1] | src[0] << 8;
				src += 2;
				disp = dst - (block & 0x0FFF) - 1;
				bytes = (block >> 12) + 3;
				while (bytes--) {
					if (remaining) {
						--remaining;
					}
					*dst++ = *disp++;
				}
			} else {
				*dst++ = *src++;
				--remaining;
			}
			block_header <<= 1;
			--blocks_remaining;
		} else {
			block_header = *src++;
			blocks_remaining = 8;
		}
	}
}

void decompress_lz77_wram(const void *restrict nonnull src,
			  void *restrict nonnull dst)
{
	assert(((size_t)src & 0x3) == 0 && "must be aligned by 4");
	assert(((size_t)dst & 0x3) == 0 && "must be aligned by 4");
	// TODO: Check to make sure dst is outside vram
	gba_lz77_decompress(src, dst);
}

void decompress_lz77_vram(const void *restrict nonnull src,
			  void *restrict nonnull dst)
{
	assert(((size_t)src & 0x3) == 0 && "must be aligned by 4");
	assert(((size_t)dst & 0x3) == 0 && "must be aligned by 4");
	// TODO: Check to make sure dst is in vram
	gba_lz77_decompress(src, dst);
}

void debug_put_char(char c)
{
	fputc(c, stdout);
}

void debug_put_str(const char *nonnull str)
{
	fputs(str, stdout);
}

void interrupt_acknowledge(uint16_t int_flag)
{
	REG_IF &= ~int_flag;
}