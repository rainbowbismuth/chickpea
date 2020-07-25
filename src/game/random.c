#include "chickpea.h"
#include "game/random.h"

static uint32_t init_counter = 0xEE2AD2F;
static struct random global_random_state = { 0 };

/*
 * TODO: Revisit this initialization, especially when I get
 *   timers going, maybe?
 */
void random_init(struct random *random)
{
	init_counter += 0x915C8C73;
	random->a = 0xE54D9699 ^ init_counter;
	random->b = 0x5FA3B2BC ^ init_counter;
	random->c = 0x52B5A326 ^ init_counter;
	random->d = 0xBFD2A478 ^ init_counter;
	random->counter = 0x56DA3F0B ^ init_counter;
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
	if (global_random_state.counter == 0) {
		random_init(&global_random_state);
	}
	return random_next(&global_random_state);
}