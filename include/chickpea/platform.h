#ifndef CHICKPEA_PLATFORM_H
#define CHICKPEA_PLATFORM_H

#ifdef BUILD_GBA
#include "chickpea/gba.h"
#else
#include "chickpea/emulated.h"
#endif

#endif //CHICKPEA_PLATFORM_H