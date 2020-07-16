#ifndef CHICKPEA_EMULATED_H
#define CHICKPEA_EMULATED_H

#include "assert.h"

extern uint16_t reg_dispcnt;
extern uint16_t reg_dispstat;
extern uint16_t reg_vcount;
extern uint16_t reg_bldcnt;
extern uint16_t reg_bldalpha;
extern uint16_t reg_bldy;
extern uint16_t reg_keyinput;
extern uint16_t reg_ie;
extern uint16_t reg_if;
extern uint32_t reg_ime;

extern uint16_t reg_bg_controls[4];
extern uint16_t reg_bg_scrolls_x[4];
extern uint16_t reg_bg_scrolls_y[4];

extern uint8_t video_ram[0x18000];

extern struct palette bg_pallete_ram[16];
extern struct palette obj_pallete_ram[16];

#define REG_DISPCNT  (reg_dispcnt)
#define REG_DISPSTAT (reg_dispstat)
#define REG_VCOUNT   (reg_vcount)
#define REG_BLDCNT   (reg_bldcnt)
#define REG_BLDALPHA (reg_bldalpha)
#define REG_BLDY     (reg_bldy)
#define REG_KEYINPUT (reg_keyinput)
#define REG_IE	     (reg_ie)
#define REG_IF	     (reg_if)
#define REG_IME	     (reg_ime)

#endif //CHICKPEA_EMULATED_H