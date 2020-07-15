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

extern struct palette bg_pallete_ram[16];
extern struct palette obj_pallete_ram[16];

#define REG_DISPCNT  (reg_dispcnt)
#define REG_DISPSTAT (reg_dispstat)
#define REG_VCOUNT   (reg_vcount)
#define REG_IME	     (reg_ime)
#define REG_IE	     (reg_ie)
#define REG_IF	     (reg_if)

#endif //CHICKPEA_EMULATED_H