#include "stdint.h"
#include "stddef.h"

#include "chickpea.h"


void our_irq_handler(void)
{
	;
}

void (*irq_handler)(void) = our_irq_handler;

uint16_t color(uint32_t red, uint32_t green, uint32_t blue)
{
	return PREP(COL_RED, red) | PREP(COL_GREEN, green) |
	       PREP(COL_BLUE, blue);
}

void wait_for_horizontal_blank()
{
	REG_IME = 0;
	REG_DISPSTAT |= DISPSTAT_HORIZONTAL_BLANK_IRQ_ENABLED;
	REG_IE |= INT_HORIZONTAL_BLANK;
	REG_IME = 1;
	halt();
	REG_IME = 0;
}

struct character_4bpp {
	uint32_t lines[8];
};

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

void write_4bpp(const struct character_4bpp *character, volatile uint32_t *vram)
{
	for (size_t i = 0; i < ARRAY_SIZE(character->lines); ++i) {
		*vram++ = character->lines[i];
	}
}

int main()
{
	REG_DISPCNT = DISPCNT_FORCED_BLANK | DISPCNT_SCREEN_DISPLAY_BG0;
	BG_PALETTE_RAM[0] = color(10, 5, 31);

	*reg_bg_control(BG0) = PREP(BGCNT_SCREEN_BLOCK, 1);
	write_4bpp(&demo_tile, &character_block_begin(0)[8]);

	volatile uint16_t *screen_block = screen_block_begin(1);
	for (size_t i = 0; i < 16 * 32; ++i) {
		screen_block[i << 1] = PREP(TILE_CHAR, 1) |
				       PREP(TILE_PALETTE, i % 4);
	}

	REG_DISPCNT &= ~DISPCNT_FORCED_BLANK;

	uint32_t frame = 0;
	while (1) {
		wait_for_horizontal_blank();
		BG_PALETTE_RAM[0] = color(10, 5, ((REG_VCOUNT >> 2) & 0b1111));

		if (REG_VCOUNT == 160) {
			frame++;
			uint32_t c = (frame >> 2) & 0x1F;
			BG_PALETTE_RAM[1] = color(22, c, 15);
			BG_PALETTE_RAM[1 + 16] = color(c, 22, 15);
			BG_PALETTE_RAM[1 + 32] = color(15, 22, c);
			BG_PALETTE_RAM[1 + 48] = color(c, c, c);

			uint32_t v = (frame >> 1) % 512;

			*reg_bg_scroll_x(BG0) = v;
			*reg_bg_scroll_y(BG0) = v;
		}
	}

	return 0;
}
