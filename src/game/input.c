#include "game/input.h"

#define NUM_BUTTONS    12
#define REPEAT_LATENCY 10

static uint32_t last_input = 0;
static uint32_t cur_input = 0;
static uint8_t repeat[NUM_BUTTONS] = { 0 };

void input_read(void)
{
	last_input = cur_input;
	cur_input = ~REG_KEYINPUT;
	for (size_t i = 0; i < NUM_BUTTONS; ++i) {
		if (input_held(1 << i)) {
			repeat[i] = (repeat[i] + 1) % REPEAT_LATENCY;
		} else {
			repeat[i] = REPEAT_LATENCY - 1;
		}
	}
}

bool input_held(uint32_t button)
{
	return (cur_input & button) != 0;
}

bool input_pressed(uint32_t button)
{
	return input_held(button) && repeat[MASK_OFFSET(button)] == 0;
}

bool input_release(uint32_t button)
{
	return !input_held(button) && (last_input & button) != 0;
}