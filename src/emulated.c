#include "chickpea.h"

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

uint16_t bg_pallete_ram[256] = { 0 };
uint16_t obj_pallete_ram[256] = { 0 };

void halt()
{
	assert(0 && "not implemented");
}