#ifndef CHICKPEA_SCREEN_H
#define CHICKPEA_SCREEN_H

#include "chickpea.h"

struct screen {
	void (*nullable update)(void);
	void (*nullable on_vertical_blank)(void);
	void (*nullable on_horizontal_blank)(void);
};

extern struct screen *nonnull current_screen;

#endif // CHICKPEA_SCREEN_H
