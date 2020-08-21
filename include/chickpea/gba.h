#ifndef CHICKPEA_GBA_H
#define CHICKPEA_GBA_H

#include "chickpea/common.h"

// "assert.h" isn't available free-standing, so we define our own.
#define static_assert _Static_assert

/*
 * We define our own assert() macro as well, that as of this moment just
 * infinite loops.
 */
#ifndef NDEBUG
#define assert(cond)                               \
	do {                                       \
		if ((cond) == 0) {                 \
			debug_put_str(#cond "\n"); \
			while (1) {                \
			}                          \
		}                                  \
	} while (0)
#else
#define assert(cond) (0)
#endif

_Noreturn void abort(void);

#define REG_DISPCNT  (*((volatile uint16_t *)0x04000000))
#define REG_DISPSTAT (*((volatile uint16_t *)0x04000004))
#define REG_VCOUNT   (*((volatile uint16_t *)0x04000006))
#define REG_BLDCNT   (*((volatile uint16_t *)0x04000050))
#define REG_BLDALPHA (*((volatile uint16_t *)0x04000052))
#define REG_BLDY     (*((volatile uint16_t *)0x04000054))
#define REG_KEYINPUT (*((volatile uint16_t *)0x04000130))
#define REG_IE	     (*((volatile uint16_t *)0x04000200))
#define REG_IF	     (*((const volatile uint16_t *)0x04000202))
#define REG_IME	     (*((volatile uint32_t *)0x04000208))

#define OAM (*((volatile struct object_attribute_mem *)0x07000000))

#define EWRAM __attribute__((section(".ewram_bss")))

#endif // CHICKPEA_GBA_H