#include "chickpea.h"
#include "chickpea/nano_unit.h"

void halt()
{
	__asm__ volatile("swi 0x2");
}

void cpu_fast_set(const void *restrict nonnull src, void *restrict nonnull dst,
		  size_t word_count)
{
	assert(((size_t)src & 0x3) == 0 && "must be aligned by 4");
	assert(((size_t)dst & 0x3) == 0 && "must be aligned by 4");
	assert(word_count % 8 == 0 && "must be multiple of 8 words");

	register size_t src_r asm("r0") = (size_t)src;
	register size_t dst_r asm("r1") = (size_t)dst;
	register size_t word_count_r asm("r2") = word_count;
	__asm__ volatile("swi 0xC"
			 : /* no output */
			 : "r"(src_r), "r"(dst_r), "r"(word_count_r));
}

void cpu_fast_fill(uint32_t src, void *nonnull dst, size_t word_count)
{
	cpu_fast_set(&src, dst, word_count | (1 << 24));
}

/*
 * TODO: Use a better implementation.
 */
void __aeabi_memcpy(uint8_t *restrict nonnull dest,
		    const uint8_t *restrict nonnull src, size_t n)
{
	for (size_t i = 0; i < n; ++i) {
		dest[i] = src[i];
	}
}

/*
 * TODO: Use a better implementation.
 */
void __aeabi_memclr(uint8_t *nonnull dest, size_t n)
{
	for (size_t i = 0; i < n; ++i) {
		dest[i] = 0;
	}
}

/*
 * TODO: Use a better implementation.
 */
void __aeabi_memclr4(uint8_t *nonnull dest, size_t n)
{
	for (size_t i = 0; i < n; ++i) {
		dest[i] = 0;
	}
}

#define BG_PALETTE_RAM	0x05000000
#define OBJ_PALETTE_RAM 0x05000200
#define VRAM_BEGIN	0x06000000

struct palette *nonnull bg_palette(uint32_t palette_idx)
{
	return (struct palette *)(size_t)(BG_PALETTE_RAM + palette_idx * 0x20);
}

struct palette *nonnull obj_palette(uint32_t palette_idx)
{
	return (struct palette *)(size_t)(OBJ_PALETTE_RAM + palette_idx * 0x20);
}

struct char_4bpp *nonnull char_block_begin(uint32_t char_block)
{
	return (struct char_4bpp *)(size_t)(VRAM_BEGIN + char_block * 0x4000);
}

uint16_t *nonnull screen_block_begin(uint32_t screen_block)
{
	return (uint16_t *)(size_t)(VRAM_BEGIN + screen_block * 0x800);
}

volatile uint16_t *nonnull reg_bg_control(enum background bg)
{
	return (volatile uint16_t *)(size_t)(0x04000008 + (bg << 1));
}

void set_bg_scroll_x(enum background bg, uint16_t scroll_x)
{
	*(volatile uint16_t *)(size_t)(0x04000010 + (bg << 2)) = scroll_x;
}

void set_bg_scroll_y(enum background bg, uint16_t scroll_y)
{
	*(volatile uint16_t *)(size_t)(0x04000012 + (bg << 2)) = scroll_y;
}

#define MGBA_DEBUG_ENABLE	((volatile uint16_t *)0x04fff780)
#define MGBA_DEBUG_OUTPUT_BEGIN ((volatile char *)0x04fff600)
#define MGBA_DEBUG_OUTPUT_END	((volatile char *)0x04fff700)
#define MGBA_DEBUG_SEND		((volatile uint16_t *)0x04fff700)

volatile char *mgba_debug_output = MGBA_DEBUG_OUTPUT_BEGIN;

void debug_put_char(char c)
{
	if (mgba_debug_output >= MGBA_DEBUG_OUTPUT_END) {
		mgba_debug_output = MGBA_DEBUG_OUTPUT_BEGIN;
		*MGBA_DEBUG_SEND = 0x104;
	}
	if (c == '\0' || c == '\n') {
		mgba_debug_output = MGBA_DEBUG_OUTPUT_BEGIN;
		*MGBA_DEBUG_SEND = 0x104;
		return;
	}
	*mgba_debug_output++ = c;
}

void debug_put_str(const char *nonnull str)
{
	while (*str != '\0') {
		debug_put_char(*str++);
	}
}

#define REG_IF_MUTABLE (*((volatile uint16_t *)0x04000202))

void interrupt_acknowledge(uint16_t int_flag)
{
	REG_IF_MUTABLE = int_flag;
}

void decompress_lz77_wram(const void *restrict nonnull src,
			  void *restrict nonnull dst)
{
	assert(((size_t)src & 0x3) == 0 && "must be aligned by 4");
	assert(((size_t)dst & 0x3) == 0 && "must be aligned by 4");
	register size_t src_r asm("r0") = (size_t)src;
	register size_t dst_r asm("r1") = (size_t)dst;
	__asm__ volatile("swi 0x11"
			 : /* no output */
			 : "r"(src_r), "r"(dst_r)
			 : "r3");
}

void decompress_lz77_vram(const void *restrict nonnull src,
			  void *restrict nonnull dst)
{
	assert(((size_t)src & 0x3) == 0 && "must be aligned by 4");
	assert(((size_t)dst & 0x3) == 0 && "must be aligned by 4");
	register size_t src_r asm("r0") = (size_t)src;
	register size_t dst_r asm("r1") = (size_t)dst;
	__asm__ volatile("swi 0x12"
			 : /* no output */
			 : "r"(src_r), "r"(dst_r)
			 : "r3");
}

extern struct nano_unit_suite test_suites[];

void main(void)
{
	*MGBA_DEBUG_ENABLE = 0xc0de;
	debug_put_str("Hello, mGBA!\n");
	nano_unit_run_suites(test_suites);

	game_init();
	while (1) {
		game_update();
	}
}