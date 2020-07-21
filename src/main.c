#include "stdint.h"
#include "stddef.h"

#include "chickpea.h"
#include "chickpea/debug_font.h"
#include "chickpea/map.h"

void our_irq_handler(void)
{
	;
}

void (*volatile irq_handler)(void) = our_irq_handler;

void wait_for_horizontal_blank()
{
	REG_IME = 0;
	REG_DISPSTAT |= DISPSTAT_HORIZONTAL_BLANK_IRQ_ENABLED;
	REG_IE |= INT_HORIZONTAL_BLANK;
	REG_IME = 1;
	halt();
	REG_IME = 0;
}

static const struct character_4bpp
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

static struct map_bit_vec highlights = {};
static struct map_byte_vec height_map = {};

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
	write_4bpp(&demo_tile, &character_block_begin(4)[1]);

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

	write_debug_msg(&default_debug_font, 2, 2, 4, 3, 3, "Hello, world!");

	REG_BLDCNT = BLDCNT_1ST_TARGET_BG1 | BLDCNT_2ND_TARGET_BG0 |
		     BLDCNT_2ND_TARGET_BD | PREP(BLDCNT_EFFECT, BLEND_ALPHA);
	REG_BLDALPHA = PREP(BLDALPHA_1ST_WEIGHT, 8) |
		       PREP(BLDALPHA_2ND_WEIGHT, 8);

	struct map_render_params map_render_params = { .char_block = 3,
						       .screen_block = 3 };
	*reg_bg_control(BG3) =
		PREP(BGCNT_CHAR_BLOCK, map_render_params.char_block) |
		PREP(BGCNT_SCREEN_BLOCK, map_render_params.screen_block);

	demo_render_tile_highlights(&map_render_params, &highlights,
				    &height_map);

	OAM.entries[0].attr_0 = PREP(OBJA0_Y, 0);
	OAM.entries[0].attr_1 = PREP(OBJA1_X, 16);
	OAM.entries[0].attr_2 = PREP(OBJA2_CHAR, 1) |
				PREP(OBJA2_PALETTE, 1);
	obj_palette(1)->color[1] = color(31, 31, 31);

	REG_DISPCNT &= ~DISPCNT_FORCED_BLANK;

	uint32_t frame = 0;
	uint16_t bg1_scroll_x = 0;
	uint16_t bg1_scroll_y = 0;
	while (1) {
		wait_for_horizontal_blank();
		bg_palette(0)->color[0] =
			color(((REG_VCOUNT >> 2) & 0b1111), 5, 10);

		if (REG_VCOUNT == 160) {
			frame++;
			uint32_t c = (frame >> 2) & 0x1F;
			bg_palette(0)->color[1] = color(22, c, 15);
			bg_palette(1)->color[1] = color(c, 22, 15);
			bg_palette(2)->color[1] = color(15, 22, c);
			bg_palette(3)->color[1] = color(c, c, c);

			demo_rotate_highlight_palette(frame);

			uint32_t v = (frame >> 1) % 512;

			set_bg_scroll_x(BG0, v);
			set_bg_scroll_y(BG0, v);

			uint16_t keyinput = ~REG_KEYINPUT;
			if (GET(KEYINPUT_UP, keyinput) != 0) {
				set_bg_scroll_y(BG1, bg1_scroll_y++);
			}
			if (GET(KEYINPUT_DOWN, keyinput) != 0) {
				set_bg_scroll_y(BG1, bg1_scroll_y--);
			}
			if (GET(KEYINPUT_LEFT, keyinput) != 0) {
				set_bg_scroll_x(BG1, bg1_scroll_x++);
			}
			if (GET(KEYINPUT_RIGHT, keyinput) != 0) {
				set_bg_scroll_x(BG1, bg1_scroll_x--);
			}

			OAM.entries[0].attr_0 = PREP(OBJA0_Y, frame);
			OAM.entries[0].attr_1 = PREP(OBJA1_X, 16);
		}
	}
}
