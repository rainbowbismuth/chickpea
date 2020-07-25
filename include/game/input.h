#ifndef CHICKPEA_INPUT_H
#define CHICKPEA_INPUT_H

#include "chickpea.h"

void input_read(void);
bool input_held(uint32_t button);
bool input_pressed(uint32_t button);
bool input_release(uint32_t button);

#endif //CHICKPEA_INPUT_H
