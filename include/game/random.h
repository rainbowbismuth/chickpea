#ifndef CHICKPEA_RANDOM_H
#define CHICKPEA_RANDOM_H

#include <stdint.h>

/*
 * Current RNG implementation is "xorwow"
 */
struct random {
	uint32_t a, b, c, d;
	uint32_t counter;
};

void random_init(struct random *random);
uint32_t random_next(struct random *random);
uint32_t random_global(void);

#endif // CHICKPEA_RANDOM_H
