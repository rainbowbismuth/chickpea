#ifndef CHICKPEA_GAME_STATE_H
#define CHICKPEA_GAME_STATE_H

#include "chickpea.h"

struct game_state {
	void (*nullable update)(void);
	void (*nullable on_vertical_blank)(void);
	void (*nullable on_horizontal_blank)(void);
};

extern struct game_state *nonnull current_screen;

#endif // CHICKPEA_GAME_STATE_H
