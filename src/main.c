#include "stdint.h"
#include "stddef.h"

#include "chickpea.h"
#include "game/debug_font.h"
#include "game/map.h"
#include "game/input.h"
#include "game/random.h"
#include "game/screen.h"

static struct vec2 bg3_scroll = { 0 };
static struct vec2 cursor_pos = { .x = 6, .y = 6 };
static uint32_t frame = 0;
static sprite_handle cursor = { 0 };
static sprite_handle soldiers[4] = { 0 };
static struct map_bit_vec highlights = { 0 };
static struct map_byte_vec height_map = { 0 };

void demo_update(void)
{
	frame++;
	input_read();

	if (input_held(KEYINPUT_BUTTON_B)) {
		if (input_pressed(KEYINPUT_UP)) {
			cursor_pos.y--;
		} else if (input_pressed(KEYINPUT_DOWN)) {
			cursor_pos.y++;
		} else if (input_pressed(KEYINPUT_LEFT)) {
			cursor_pos.x--;
		} else if (input_pressed(KEYINPUT_RIGHT)) {
			cursor_pos.x++;
		}
	} else {
		if (input_held(KEYINPUT_UP)) {
			bg3_scroll.y++;
			set_bg_scroll_y(BG3, bg3_scroll.y);
		}
		if (input_held(KEYINPUT_DOWN)) {
			bg3_scroll.y--;
			set_bg_scroll_y(BG3, bg3_scroll.y);
		}
		if (input_held(KEYINPUT_LEFT)) {
			bg3_scroll.x++;
			set_bg_scroll_x(BG3, bg3_scroll.x);
		}
		if (input_held(KEYINPUT_RIGHT)) {
			bg3_scroll.x--;
			set_bg_scroll_x(BG3, bg3_scroll.x);
		}
	}

	demo_move_cursor(&height_map, cursor, cursor_pos, bg3_scroll);

	demo_move_soldier(&height_map, soldiers[0],
			  (struct vec2){ .x = 4, .y = 4 }, bg3_scroll);
	demo_move_soldier(&height_map, soldiers[1],
			  (struct vec2){ .x = 6, .y = 4 }, bg3_scroll);
	demo_move_soldier(&height_map, soldiers[2],
			  (struct vec2){ .x = 4, .y = 6 }, bg3_scroll);
	demo_move_soldier(&height_map, soldiers[3],
			  (struct vec2){ .x = 6, .y = 6 }, bg3_scroll);

	sprite_build_oam_buffer();
}

void demo_on_horizontal_blank(void)
{
	bg_palette(0)->color[0] = color(((REG_VCOUNT >> 2) & 0b1111), 5, 10);
}

void demo_on_vertical_blank(void)
{
	if (frame % 30 == 0) {
		set_bg_scroll_x(BG1, random_global());
		set_bg_scroll_y(BG1, random_global());
	}

	uint32_t c = (frame >> 2) & 0x0F;

	bg_palette(0)->color[1] = color(11, c, 7);
	bg_palette(1)->color[1] = color(c, 11, 7);
	bg_palette(2)->color[1] = color(7, 11, c);
	bg_palette(3)->color[1] = color(c, c, c);
	demo_rotate_highlight_palette(frame);

	uint32_t v = (frame >> 1) % 512;

	set_bg_scroll_x(BG0, v);
	set_bg_scroll_y(BG0, v);

	uint32_t walk_c[4] = { 0, 1, 2, 1 };
	uint32_t unit_frame = walk_c[(frame / 12) % ARRAY_SIZE(walk_c)];
	demo_soldier_frame(soldiers[0], FACING_EAST, unit_frame);
	demo_soldier_frame(soldiers[1], FACING_SOUTH, unit_frame);
	demo_soldier_frame(soldiers[2], FACING_NORTH, unit_frame);
	demo_soldier_frame(soldiers[3], FACING_WEST, unit_frame);
	sprite_commit_buffer_to_oam();
//	if (frame % 600 == 0) {
//		debug_put_str("time ");
//		debug_put_u32(REG_VCOUNT - 160);
//		debug_put_str("\n");
//	}
}

struct screen *nonnull current_screen = &(struct screen){
	.update = &demo_update,
	.on_horizontal_blank = &demo_on_horizontal_blank,
	.on_vertical_blank = &demo_on_vertical_blank,
};

void our_irq_handler(void)
{
	if (REG_IF & INT_HORIZONTAL_BLANK &&
	    current_screen->on_horizontal_blank) {
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

static const struct char_4bpp
	demo_tile = { .lines = {
			      0b00010001000100010001000100010001u,
			      0b00010000000000000000000000000001u,
			      0b00010000000000010001000000000001u,
			      0b00010000000000010001000000000001u,
			      0b00010000000000010001000000000001u,
			      0b00010000000000010001000000000001u,
			      0b00010000000000000000000000000001u,
			      0b00010001000100010001000100010001u,
		      } };

void game_main(void)
{
	REG_DISPCNT = DISPCNT_FORCED_BLANK |
		      DISPCNT_OBJ_ONE_DIMENSIONAL_MAPPING |
		      DISPCNT_SCREEN_DISPLAY_BG0 | DISPCNT_SCREEN_DISPLAY_BG1 |
		      DISPCNT_SCREEN_DISPLAY_BG2 | DISPCNT_SCREEN_DISPLAY_BG3 |
		      DISPCNT_SCREEN_DISPLAY_OBJ;

	bg_palette(0)->color[0] = color(10, 5, 31);

	*reg_bg_control(BG0) = PREP(BGCNT_SCREEN_BLOCK, 1) |
			       PREP(BGCNT_PRIORITY, 1);

	write_4bpp(&demo_tile, &character_block_begin(0)[1]);

	*reg_bg_control(BG1) = PREP(BGCNT_CHAR_BLOCK, 2) |
			       PREP(BGCNT_SCREEN_BLOCK, 2);

	*reg_bg_control(BG2) = PREP(BGCNT_CHAR_BLOCK, 2) |
			       PREP(BGCNT_SCREEN_BLOCK, 2) |
			       PREP(BGCNT_PRIORITY, 1);

	set_bg_scroll_x(BG2, -20);
	set_bg_scroll_y(BG2, -20);

	volatile uint16_t *tiles = screen_block_begin(1);
	for (size_t i = 0; i < 16 * 32; ++i) {
		if (i % 7 == 0) {
			continue;
		}
		tiles[i << 1] = PREP(TILE_CHAR, 1) | PREP(TILE_PALETTE, i % 4);
	}

	struct debug_font default_debug_font = {
		.characters = debug_font_4bpp,
		.palette = &debug_font_pal,
	};

	write_debug_msg(&default_debug_font, 2, 2, 4, 3, 3, "Hello, world!");

	REG_BLDCNT = BLDCNT_1ST_TARGET_BG3 | BLDCNT_1ST_TARGET_BG1 |
		     BLDCNT_2ND_TARGET_BG0 | BLDCNT_2ND_TARGET_BD |
		     PREP(BLDCNT_EFFECT, BLEND_ALPHA);
	REG_BLDALPHA = PREP(BLDALPHA_1ST_WEIGHT, 8) |
		       PREP(BLDALPHA_2ND_WEIGHT, 8);

	struct map_render_params map_render_params = { .char_block = 3,
						       .screen_block = 3 };
	*reg_bg_control(BG3) =
		PREP(BGCNT_CHAR_BLOCK, map_render_params.char_block) |
		PREP(BGCNT_SCREEN_BLOCK, map_render_params.screen_block);

	demo_init();
	demo_render_tile_highlights(&map_render_params, &highlights,
				    &height_map);

	cursor = demo_alloc_cursor();
	sprite_ref(cursor)->enabled = true;
	sprite_ref(cursor)->order = 100;

	for (size_t i = 0; i < ARRAY_SIZE(soldiers); ++i) {
		soldiers[i] = demo_alloc_soldier();
		sprite_ref(soldiers[i])->enabled = true;
	}

	REG_DISPCNT &= ~DISPCNT_FORCED_BLANK;

	REG_IME = 0;
	REG_DISPSTAT |= DISPSTAT_HORIZONTAL_BLANK_IRQ_ENABLED |
			DISPSTAT_VERTICAL_BLANK_IRQ_ENABLED;
	REG_IE |= INT_HORIZONTAL_BLANK | INT_VERTICAL_BLANK;
	REG_IME = 1;

	while (1) {
		if (REG_VCOUNT == 0 &&
		    !GET(DISPSTAT_HORIZONTAL_BLANK, REG_DISPSTAT) &&
		    current_screen->update) {
			current_screen->update();
		}
		halt();
	}
}
