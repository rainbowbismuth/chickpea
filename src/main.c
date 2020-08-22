#include "stddef.h"
#include "stdint.h"

#include "chickpea.h"
#include "game/debug_font.h"
#include "game/font.h"
#include "game/game_state.h"
#include "game/input.h"
#include "game/map.h"
#include "game/resource.h"
#include "game/text_box.h"
#include "lua/alloc.h"

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

static struct lua_State *lua = NULL;
static struct lua_State *coro = NULL;
static bool coro_started = false;
extern struct resource lua_scripts;

static volatile bool run_update = true;
static struct vec2 bg_scroll = { 0 };
static struct vec2 cursor_pos = { .x = 11, .y = 6 };
static uint32_t frame = 0;
static sprite_handle cursor = { 0 };
static sprite_handle pointer = { 0 };
static sprite_handle soldiers[4] = { 0 };
static sprite_handle height_msg = { 0 };
struct map_render_params map_render_params = { .char_block = 3,
					       .screen_low = 12,
					       .screen_high = 13 };
static struct debug_font demo_font = { 0 };

extern struct resource fonts_bismuth_font_4bpp;
extern struct resource fonts_bismuth_font_width;
static struct font bismuth = {
	.letter_spacing = 1,
	.tall = true,
};
static struct font mono = {
	.letter_spacing = 0,
	.tall = false,
};

extern struct resource fonts_bismuth_font_pal;

static struct sprite_object_def height_msg_objs[1] = { {
	.x_offset = 0,
	.y_offset = 0,
	.shape = OBJ_SHAPE_HORIZONTAL,
	.size = OBJ_SIZE_16,
} };

static struct sprite_template height_msg_template = {
	.palette = 11,
	.mode = OBJ_MODE_NORMAL,
	.num_objects = ARRAY_SIZE(height_msg_objs),
	.objects = height_msg_objs
};

extern struct resource interface_speech_bubble_4bpp;
extern struct resource interface_speech_bubble_tiles;
extern struct resource portraits_Bjin_8bpp;
extern struct resource portraits_Bjin_pals;

static struct sprite_object_def portrait_objs[5] = {
	{ .x_offset = 0,
	  .y_offset = 0,
	  .shape = OBJ_SHAPE_VERTICAL,
	  .size = OBJ_SIZE_64 },
	{ .x_offset = 32,
	  .y_offset = 0,
	  .shape = OBJ_SHAPE_VERTICAL,
	  .size = OBJ_SIZE_32 },
	{ .x_offset = 32,
	  .y_offset = 32,
	  .shape = OBJ_SHAPE_VERTICAL,
	  .size = OBJ_SIZE_32 },
	{ .x_offset = 0,
	  .y_offset = 64,
	  .shape = OBJ_SHAPE_HORIZONTAL,
	  .size = OBJ_SIZE_32 },
	{ .x_offset = 32,
	  .y_offset = 64,
	  .shape = OBJ_SHAPE_SQUARE,
	  .size = OBJ_SIZE_16 },
};

static struct sprite_template portrait_template = {
	.mode = OBJ_MODE_NORMAL,
	.num_objects = ARRAY_SIZE(portrait_objs),
	.objects = portrait_objs,
	.colors_256 = true,
};

static uint32_t text_select = 0;

static struct text_config bismuth_config = { 0 };
static struct text_renderer text_renderer = { 0 };

static struct text_config mono_config = { 0 };
static struct text_renderer mono_text_renderer = { 0 };

void move_cursor_pos_bounded(int16_t x, int16_t y)
{
	uint8_t attr = demo_map.attributes->bytes[y][x];
	if (~attr & MAP_ATTR_WALK) {
		return;
	}
	cursor_pos.x = x;
	cursor_pos.y = y;
}

void demo_update(void)
{
	frame++;
	input_read();

	if (coro) {
		int status = lua_status(coro);
		if (coro_started && status != LUA_YIELD) {
			goto end_coro;
		}
		int n_results = 0;
		int err = lua_resume(coro, NULL, 0, &n_results);
		if (err) {
			debug_put_str(lua_tolstring(coro, -1, NULL));
		}
		assert(!n_results);
		coro_started = true;
	}
end_coro:

	if (!input_held(KEYINPUT_BUTTON_B)) {
		if (input_pressed(KEYINPUT_UP)) {
			move_cursor_pos_bounded(cursor_pos.x, cursor_pos.y - 1);
		} else if (input_pressed(KEYINPUT_DOWN)) {
			move_cursor_pos_bounded(cursor_pos.x, cursor_pos.y + 1);
		} else if (input_pressed(KEYINPUT_LEFT)) {
			move_cursor_pos_bounded(cursor_pos.x - 1, cursor_pos.y);
		} else if (input_pressed(KEYINPUT_RIGHT)) {
			move_cursor_pos_bounded(cursor_pos.x + 1, cursor_pos.y);
		}
	} else {
		if (input_held(KEYINPUT_UP)) {
			bg_scroll.y++;
		}
		if (input_held(KEYINPUT_DOWN)) {
			bg_scroll.y--;
		}
		if (input_held(KEYINPUT_LEFT)) {
			bg_scroll.x++;
		}
		if (input_held(KEYINPUT_RIGHT)) {
			bg_scroll.x--;
		}
	}
	demo_move_cursor(&demo_map, cursor, cursor_pos, bg_scroll);
	demo_move_pointer(&demo_map, pointer, cursor_pos, bg_scroll, frame);

	demo_move_character(&demo_map, soldiers[0],
			    (struct vec2){ .x = 8, .y = 9 }, bg_scroll);
	demo_move_character(&demo_map, soldiers[1],
			    (struct vec2){ .x = 13, .y = 9 }, bg_scroll);
	demo_move_character(&demo_map, soldiers[2],
			    (struct vec2){ .x = 7, .y = 10 }, bg_scroll);
	demo_move_character(&demo_map, soldiers[3],
			    (struct vec2){ .x = 11, .y = 11 }, bg_scroll);

	uint32_t walk_c[4] = { 0, 1, 2, 1 };
	uint32_t unit_frame = walk_c[(frame / 20) % ARRAY_SIZE(walk_c)];
	demo_character_frame(soldiers[0], FACING_EAST, unit_frame);
	demo_character_frame(soldiers[1], FACING_SOUTH, unit_frame);
	demo_character_frame(soldiers[2], FACING_NORTH, unit_frame);
	demo_character_frame(soldiers[3], FACING_WEST, unit_frame);

	sprite_build_oam_buffer();
}

void demo_on_horizontal_blank(void)
{
	uint16_t vcount = REG_VCOUNT;
	if (vcount > 160) {
		return;
	}
	if (vcount == 160) {
		vcount = 0;
	}
	bg_palette(0)->color[0] = color(((vcount >> 4) & 0b1111) + 16, 15, 25);
}

void demo_on_vertical_blank(void)
{
	//	set_bg_scroll_x(BG0, bg_scroll.x);
	//	set_bg_scroll_y(BG0, bg_scroll.y);
	//	set_bg_scroll_x(BG1, bg_scroll.x);
	//	set_bg_scroll_y(BG1, bg_scroll.y);
	set_bg_scroll_x(BG2, bg_scroll.x);
	set_bg_scroll_y(BG2, bg_scroll.y);
	set_bg_scroll_x(BG3, bg_scroll.x);
	set_bg_scroll_y(BG3, bg_scroll.y);

	demo_rotate_highlight_palette(frame);

	sprite_execute_frame_copies();
	sprite_commit_buffer_to_oam();

	if (sprite_exists(height_msg)) {
		sprite_drop(height_msg);
	}
	char msg[3] = "0h";
	msg[0] = '0' + demo_map.height->bytes[cursor_pos.y][cursor_pos.x];
	height_msg =
		write_debug_msg_sprite(&demo_font, &height_msg_template, msg);
	sprite_ref(height_msg)->enabled = true;
	sprite_ref(height_msg)->pos = (struct vec2){ .x = 27 * 8, .y = 8 };

	struct text_renderer *rend = text_select ? &mono_text_renderer
						 : &text_renderer;

	if (frame % 7 == 0) {
		if (text_renderer_at_end(rend) && frame % 70 == 0) {
			text_renderer_clear(rend);
			text_select ^= 1;
		} else {
			text_renderer_next_char(rend);
		}
	}

	run_update = true;
}

struct game_state *nonnull current_screen = &(struct game_state){
	.update = &demo_update,
	.on_horizontal_blank = &demo_on_horizontal_blank,
	.on_vertical_blank = &demo_on_vertical_blank,
};

void our_irq_handler(void)
{
	if (REG_IF & INT_HORIZONTAL_BLANK
	    && current_screen->on_horizontal_blank) {
		current_screen->on_horizontal_blank();
		interrupt_acknowledge(INT_HORIZONTAL_BLANK);
		return;
	}
	if (REG_IF & INT_VERTICAL_BLANK && current_screen->on_vertical_blank) {
		current_screen->on_vertical_blank();
		interrupt_acknowledge(INT_VERTICAL_BLANK);
		return;
	}
}

void (*volatile irq_handler)(void) = our_irq_handler;

static struct map_bit_vec highlights = { 0 };

void game_init(void)
{
	REG_DISPCNT = DISPCNT_FORCED_BLANK | DISPCNT_OBJ_ONE_DIMENSIONAL_MAPPING
		    | DISPCNT_SCREEN_DISPLAY_BG0 | DISPCNT_SCREEN_DISPLAY_BG1
		    | DISPCNT_SCREEN_DISPLAY_BG2 | DISPCNT_SCREEN_DISPLAY_BG3
		    | DISPCNT_SCREEN_DISPLAY_OBJ;

	bg_palette(0)->color[0] = color(10, 5, 31);

	*reg_bg_control(BG0) =
		PREP(BGCNT_CHAR_BLOCK, map_render_params.char_block)
		| PREP(BGCNT_SCREEN_BLOCK, map_render_params.screen_low)
		| PREP(BGCNT_PRIORITY, 1);

	*reg_bg_control(BG1) =
		PREP(BGCNT_CHAR_BLOCK, map_render_params.char_block)
		| PREP(BGCNT_SCREEN_BLOCK, map_render_params.screen_high)
		| PREP(BGCNT_PRIORITY, 0);

	*reg_bg_control(BG2) = PREP(BGCNT_SCREEN_BLOCK, 8)
			     | PREP(BGCNT_PRIORITY, 3)
			     | PREP(BGCNT_SCREEN_SIZE, 1);

	*reg_bg_control(BG3) = PREP(BGCNT_SCREEN_BLOCK, 10)
			     | PREP(BGCNT_PRIORITY, 2)
			     | PREP(BGCNT_SCREEN_SIZE, 1);

	//	REG_BLDCNT = BLDCNT_1ST_TARGET_BG0 | BLDCNT_1ST_TARGET_BG1 |
	//		     BLDCNT_2ND_TARGET_BG0 | BLDCNT_2ND_TARGET_BG1 |
	//		     BLDCNT_2ND_TARGET_BG2 | BLDCNT_2ND_TARGET_BG3 |
	//		     BLDCNT_2ND_TARGET_OBJ | BLDCNT_2ND_TARGET_BD |
	//		     PREP(BLDCNT_EFFECT, BLEND_ALPHA);
	//	REG_BLDALPHA = PREP(BLDALPHA_1ST_WEIGHT, 5) |
	//		       PREP(BLDALPHA_2ND_WEIGHT, 11);

	demo_font.characters = resource_data(&fonts_debug_font_4bpp);
	demo_font.palette = resource_data(&fonts_debug_font_pal);

	bismuth.characters =
		(struct char_4bpp *)resource_data(&fonts_bismuth_font_4bpp);
	bismuth.widths = (uint8_t *)resource_data(&fonts_bismuth_font_width);

	mono.characters =
		(struct char_4bpp *)resource_data(&fonts_debug_font_4bpp);
	mono.widths = NULL;

	demo_init();

	cursor = demo_alloc_cursor();
	sprite_ref(cursor)->enabled = true;

	pointer = demo_alloc_pointer();
	sprite_ref(pointer)->enabled = true;
	sprite_ref(pointer)->priority[0] = 2;

	for (size_t i = 0; i < ARRAY_SIZE(soldiers); ++i) {
		soldiers[i] = demo_alloc_character();
		sprite_ref(soldiers[i])->enabled = true;
	}

	sprite_handle bjin = sprite_alloc(&portrait_template);
	resource_copy_to_vram(&portraits_Bjin_pals, (void *)obj_palette(6));
	sprite_ref(bjin)->enabled = true;
	sprite_ref(bjin)->pos.y -= 4;

	// TODO: This is a bit silly..
	sprite_queue_frame_copy(bjin, resource_data(&portraits_Bjin_8bpp));
	sprite_execute_frame_copies();

	for (size_t y = 0; y < MAP_HEIGHT; ++y) {
		for (size_t x = 0; x < MAP_WIDTH; ++x) {
			uint8_t attr = demo_map.attributes->bytes[y][x];
			if (~attr & MAP_ATTR_WALK) {
				continue;
			}
			map_bit_vec_set(&highlights,
					(struct vec2){ .x = x, .y = y });
		}
	}

	bismuth_config = (struct text_config){
		.chars = char_block_begin(map_render_params.char_block) + 32,
		.screen = screen_block_begin(map_render_params.screen_high)
			+ (32 * 3) + 8,
		.char_block = map_render_params.char_block,
		.palette = 2
	};

	mono_config = bismuth_config;

	resource_copy_to_vram(&fonts_bismuth_font_pal, (void *)bg_palette(2));
	bg_palette(2)->color[0] = color(31, 31, 31);

	struct text_box_graphics speech_bubble_gfx = {
		.chars = resource_data(&interface_speech_bubble_4bpp),
		.length = interface_speech_bubble_4bpp.length,
		.tiles = resource_data(&interface_speech_bubble_tiles),
		.height = 8,
	};

	struct text_box_config text_box = {
		.gfx = &speech_bubble_gfx,
		.from_top = 2,
		.palette = 2,
		.char_block = map_render_params.char_block,
		.screen_block = map_render_params.screen_low,
		.align_right = false,
		.width = 24,
	};

	text_box_draw(&text_box);
	text_renderer_init(&text_renderer, &bismuth, &bismuth_config,
			   "Letter, by letter,\n"
			   "I speak!\06\06\06 No cute\n"
			   "noises yet though.\06\06\06");
	text_renderer_init(&mono_text_renderer, &mono, &mono_config,
			   "This is a tiny\n"
			   "monospace font\n"
			   "and it barely\n"
			   "fits to be\n"
			   "honest.\06\06\06");

	//	demo_render_tile_highlights(&demo_map, &map_render_params,
	//&highlights);

	lua = luaL_newstate();
	luaL_openlibs(lua);
	lua_alloc_debug_mem_use();
	int32_t res = luaL_loadbuffer(lua, resource_data(&lua_scripts),
				      lua_scripts.length, "");
	if (res) {
		debug_put_str("luaL_loadbuffer: ");
		debug_put_u32(res);
		debug_put_char('\n');
		abort();
	}

	lua_call(lua, 0, 1);
	coro = lua_tothread(lua, -1);
	assert(coro);

	lua_alloc_debug_mem_use();

	REG_DISPCNT &= ~DISPCNT_FORCED_BLANK;

	REG_IME = 0;
	REG_DISPSTAT |= DISPSTAT_HORIZONTAL_BLANK_IRQ_ENABLED
		      | DISPSTAT_VERTICAL_BLANK_IRQ_ENABLED;
	REG_IE |= INT_HORIZONTAL_BLANK | INT_VERTICAL_BLANK;
	REG_IME = 1;
}

bool game_update(void)
{
	bool updated = false;
	if (REG_VCOUNT == 0 && run_update && current_screen->update) {
		current_screen->update();
		run_update = false;
		updated = true;
	}
	halt();
	return updated;
}