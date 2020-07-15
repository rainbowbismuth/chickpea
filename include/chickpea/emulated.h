#ifndef CHICKPEA_EMULATED_H
#define CHICKPEA_EMULATED_H

#include "assert.h"

extern uint16_t reg_dispcnt;
extern uint16_t reg_dispstat;
extern uint16_t reg_vcount;
extern uint32_t reg_ime;
extern uint16_t reg_ie;
extern uint16_t reg_if;

extern uint16_t reg_bg_controls[4];
extern uint16_t reg_bg_scrolls_x[4];
extern uint16_t reg_bg_scrolls_y[4];

extern uint8_t video_ram[0x18000];

extern uint16_t bg_pallete_ram[256];
extern uint16_t obj_pallete_ram[256];

#define REG_DISPCNT  (reg_dispcnt)
#define REG_DISPSTAT (reg_dispstat)
#define REG_VCOUNT   (reg_vcount)
#define REG_IME	     (reg_ime)
#define REG_IE	     (reg_ie)
#define REG_IF	     (reg_if)

#define BG_PALETTE_RAM ((volatile uint16_t *)&bg_pallete_ram)

static volatile uint32_t *character_block_begin(uint32_t char_block)
{
	return (volatile uint32_t *)(&video_ram[char_block * 0x4000]);
}

static volatile uint16_t *screen_block_begin(uint32_t screen_block)
{
	return (volatile uint16_t *)(&video_ram[screen_block * 0x800]);
}

static volatile uint16_t *reg_bg_control(enum background bg)
{
	return (volatile uint16_t *)&reg_bg_controls[bg];
}

static volatile uint16_t *reg_bg_scroll_x(enum background bg)
{
	return (volatile uint16_t *)&reg_bg_scrolls_x[bg];
}

static volatile uint16_t *reg_bg_scroll_y(enum background bg)
{
	return (volatile uint16_t *)&reg_bg_scrolls_y[bg];
}

#endif //CHICKPEA_EMULATED_H