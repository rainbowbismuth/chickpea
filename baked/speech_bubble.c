#include "game/resource.h"

static const uint8_t interface_speech_bubble_4bpp_bytes[] __attribute__((aligned(4))) = {
	0x10, 0x00, 0x01, 0x00, 0x2A, 0x00, 0x00, 0xE0, 0x01, 0x00, 0xE0, 0x01, 
	0x00, 0x30, 0x01, 0x88, 0x00, 0x88, 0x88, 0x88, 0x77, 0x77, 0x77, 0x77, 
	0x22, 0x06, 0x22, 0x22, 0x22, 0x33, 0x33, 0x30, 0x01, 0x00, 0x16, 0x00, 
	0x80, 0x50, 0x01, 0x88, 0x08, 0x00, 0x00, 0x77, 0x87, 0x08, 0x00, 0x00, 
	0x22, 0x22, 0x87, 0x00, 0x33, 0x33, 0x72, 0x05, 0x08, 0x33, 0x33, 0x23, 
	0x87, 0x00, 0x22, 0x33, 0xE0, 0x01, 0x55, 0x33, 0xA0, 0x01, 0x72, 0x00, 
	0x03, 0x23, 0x00, 0x03, 0x33, 0xE0, 0x01, 0x12, 0x33, 0x33, 0x33, 0x00, 
	0x52, 0x00, 0x87, 0x00, 0x03, 0x72, 0xB3, 0x00, 0x08, 0x23, 0x10, 0x03, 
	0x00, 0x0C, 0x33, 0x82, 0x00, 0x03, 0x00, 0x10, 0x3E, 0x33, 0x73, 0x00, 
	0x03, 0x00, 0x14, 0x00, 0x66, 0x00, 0x6F, 0x10, 0x03, 0x82, 0xD7, 0x00, 
	0x03, 0x10, 0x77, 0x73, 0x00, 0x03, 0x23, 0x90, 0x03, 0x10, 0x87, 0x10, 
	0x03, 0x40, 0x82, 0xD0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

struct resource interface_speech_bubble_4bpp = {
	.length = 256,
	.lz77 = true,
	.data = interface_speech_bubble_4bpp_bytes,
};

static const uint8_t interface_speech_bubble_pal_bytes[] __attribute__((aligned(4))) = {
	0xFF, 0x7F, 0x86, 0x10, 0x54, 0x4A, 0x39, 0x5F, 0x98, 0x46, 0xD3, 0x39, 
	0x90, 0x39, 0x29, 0x2D, 0x00, 0x00, 0xB4, 0x14, 0xF5, 0x6E, 0x2E, 0x56, 
	0x4E, 0x08, 0xDC, 0x39, 0xCE, 0x18, 0x00, 0x00, 
};

struct resource interface_speech_bubble_pal = {
	.length = 32,
	.lz77 = false,
	.data = interface_speech_bubble_pal_bytes,
};

static const uint8_t interface_speech_bubble_tiles_bytes[] __attribute__((aligned(4))) = {
	0x10, 0x40, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x04, 0x02, 0x00, 0x00, 
	0x0C, 0x01, 0x03, 0x00, 0x03, 0x0C, 0x04, 0x00, 0x05, 0x00, 0x05, 0xAB, 
	0x10, 0x01, 0x06, 0x40, 0x07, 0x07, 0x50, 0x07, 0x08, 0x10, 0x05, 0x00, 
	0x17, 0x80, 0x20, 0x07, 0x04, 0x08, 0x05, 0x08, 0x01, 0x0C, 0x01, 0x00, 
	0x0C, 0x02, 0x08, 0x00, 0x0C, 0x00, 0x00, 0x00, 
};

struct resource interface_speech_bubble_tiles = {
	.length = 64,
	.lz77 = true,
	.data = interface_speech_bubble_tiles_bytes,
};
