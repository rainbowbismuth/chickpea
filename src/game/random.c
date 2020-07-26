#include "game/random.h"

static struct random global_random = { .a = 0xE54D9699,
				       .b = 0x5FA3B2BC,
				       .c = 0x52B5A326,
				       .d = 0xBFD2A478,
				       .counter = 0x56DA3F0B };

void random_init(struct random *random)
{
	random->a = random_global();
	random->b = random_global();
	random->c = random_global();
	random->d = random_global();
	random->counter = random_global();
}

uint32_t random_next(struct random *random)
{
	uint32_t t = random->d;

	uint32_t s = random->a;
	random->d = random->c;
	random->c = random->b;
	random->b = s;

	t ^= t >> 2;
	t ^= t << 1;
	t ^= s ^ (s << 4);
	random->a = t;

	random->counter += 362437;
	return t + random->counter;
}

uint32_t random_global(void)
{
	return random_next(&global_random);
}