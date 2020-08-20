#include "game/resource.h"

static const uint8_t characters_bjin_4bpp_bytes[] __attribute__((aligned(4))) = {
	0x10, 0x00, 0x06, 0x00, 0x2A, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0x01, 
	0x22, 0x00, 0x02, 0xBC, 0x00, 0x00, 0x20, 0xBC, 0xBB, 0x00, 0xC2, 0xBB, 
	0xBB, 0x20, 0x00, 0xB2, 0x00, 0x03, 0x20, 0x54, 0x77, 0x00, 0x20, 0x40, 
	0xCC, 0x00, 0x03, 0x26, 0xC7, 0x00, 0x20, 0x21, 0x27, 0x00, 0x00, 0x20, 
	0x74, 0x27, 0x00, 0xE2, 0x78, 0x77, 0xA0, 0x00, 0x2B, 0x00, 0xD0, 0x01, 
	0x22, 0x02, 0x00, 0x00, 0xCB, 0x00, 0x2C, 0x00, 0x00, 0xBB, 0xCB, 0x02, 
	0x00, 0x3B, 0x00, 0xB5, 0x02, 0x00, 0x73, 0x54, 0x02, 0x00, 0x47, 0x08, 
	0x45, 0x02, 0x00, 0x57, 0x00, 0x07, 0x4C, 0x47, 0x02, 0x01, 0x00, 0x76, 
	0x87, 0x02, 0x00, 0x71, 0x38, 0x00, 0x13, 0x00, 0x28, 0x00, 0x00, 0x20, 
	0xD7, 0x8E, 0x47, 0x72, 0x00, 0xE5, 0x7D, 0x88, 0x72, 0x73, 0xEE, 0x77, 
	0x20, 0x00, 0x42, 0xE8, 0x74, 0x00, 0x20, 0x88, 0xEE, 0x00, 0x00, 0x20, 
	0x8C, 0x48, 0x00, 0xC2, 0xCE, 0xCE, 0x00, 0x00, 0xE2, 0xED, 0xCE, 0x00, 
	0x20, 0xCD, 0xDD, 0x00, 0x00, 0x00, 0xC2, 0xFE, 0x20, 0x22, 0xD2, 0xE3, 
	0x22, 0x00, 0x22, 0xE3, 0x34, 0x22, 0x22, 0x47, 0x47, 0x22, 0x02, 0x22, 
	0xC2, 0x2C, 0x20, 0x22, 0x22, 0x10, 0xA3, 0x22, 0x00, 0x83, 0x2E, 0x00, 
	0x00, 0xD8, 0x3D, 0x02, 0x00, 0x00, 0xE4, 0x7E, 0x25, 0x00, 0x3E, 0x74, 
	0x77, 0x02, 0x00, 0x7E, 0x34, 0x37, 0x27, 0x73, 0x99, 0x74, 0x27, 0x05, 
	0xED, 0x9E, 0x22, 0x02, 0xE8, 0x00, 0x83, 0xEE, 0x00, 0x03, 0x41, 0xDD, 
	0x00, 0x03, 0x2E, 0x22, 0x22, 0x02, 0x2C, 0x00, 0x33, 0x57, 0x22, 0x70, 
	0x01, 0x02, 0x10, 0x41, 0x00, 0xE0, 0x01, 0xF0, 0xFB, 0xF0, 0xFB, 0xEF, 
	0x30, 0xFB, 0x10, 0xBB, 0x00, 0x2F, 0x00, 0x90, 0x01, 0xF0, 0xFB, 0x00, 
	0xE7, 0xF0, 0xFB, 0xD4, 0x20, 0xFB, 0x10, 0xBB, 0x20, 0x00, 0xFB, 0x20, 
	0x10, 0xFB, 0x87, 0xEE, 0x10, 0x74, 0x00, 0x72, 0x00, 0xFB, 0xC2, 0x83, 
	0x48, 0x00, 0x00, 0x20, 0x3D, 0x8C, 0x00, 0xD2, 0xEE, 0xEC, 0x00, 0x30, 
	0xC2, 0xEE, 0x00, 0x0F, 0x00, 0x07, 0x20, 0xEE, 0xC2, 0x20, 0x02, 0x22, 
	0x53, 0x22, 0x22, 0x72, 0xCC, 0x00, 0xB6, 0x22, 0xE0, 0x20, 0x01, 0x50, 
	0xFF, 0x20, 0xFB, 0x3E, 0x27, 0x00, 0xEE, 0x73, 0x80, 0x00, 0x03, 0x38, 
	0x57, 0x02, 0x83, 0x37, 0x77, 0x02, 0x01, 0xDC, 0x37, 0x37, 0x02, 0xDC, 
	0x3E, 0x73, 0x00, 0x03, 0x00, 0x47, 0x02, 0xEE, 0xCE, 0x22, 0x00, 0xFE, 
	0xCD, 0x00, 0x02, 0x00, 0xEC, 0xEE, 0x22, 0x02, 0xE2, 0x3E, 0x00, 0x22, 
	0x22, 0xD2, 0x43, 0x22, 0x22, 0x32, 0xC5, 0xAF, 0x00, 0x4A, 0x2C, 0xF0, 
	0xFF, 0x00, 0x70, 0x01, 0x00, 0x19, 0xF1, 0x03, 0xE1, 0x03, 0x5F, 0x20, 
	0xF1, 0xFF, 0x00, 0x00, 0x01, 0xF1, 0x03, 0x00, 0xEF, 0xF1, 0x03, 0x21, 
	0x03, 0x23, 0x00, 0x20, 0x01, 0x43, 0x20, 0x7D, 0x88, 0x00, 0xE3, 0x00, 
	0x5B, 0xA0, 0x01, 0x03, 0xC2, 0x01, 0x03, 0xD2, 0x83, 0x48, 0x20, 0xDC, 
	0x00, 0xCE, 0x8C, 0x20, 0xED, 0xEE, 0xEE, 0x20, 0xEC, 0x00, 0xEE, 0xDC, 
	0x20, 0xC2, 0xEE, 0xC2, 0x22, 0x5E, 0x0B, 0x5E, 0x22, 0x22, 0x37, 0x01, 
	0xC5, 0xC2, 0x31, 0xC5, 0x50, 0xFF, 0xC8, 0x11, 0x43, 0x21, 0x03, 0x7E, 
	0x02, 0x01, 0x03, 0x23, 0x00, 0x8E, 0x0A, 0x77, 0x27, 0x00, 0x78, 0x00, 
	0x07, 0x78, 0x00, 0x63, 0x8E, 0x00, 0xC8, 0x02, 0x00, 0xCE, 0xCE, 0x2C, 
	0x00, 0xED, 0x81, 0x02, 0x04, 0xDC, 0xDF, 0xEE, 0x02, 0x22, 0xEC, 0x02, 
	0x42, 0x8B, 0x40, 0xFC, 0x22, 0x22, 0x22, 0xF0, 0xFF, 0x00, 0x60, 0x01, 
	0x00, 0x19, 0x39, 0x22, 0xCC, 0x00, 0xEF, 0x40, 0xFF, 0x00, 0x07, 0x20, 
	0xB4, 0x00, 0x03, 0x50, 0x45, 0x01, 0x13, 0x57, 0x00, 0x03, 0x74, 0xB5, 
	0x00, 0x00, 0x31, 0x72, 0xC4, 0x00, 0xBC, 0xF0, 0xFF, 0x00, 0x00, 0x00, 
	0x10, 0xFF, 0x48, 0xBB, 0x40, 0xFF, 0xBB, 0xBB, 0x00, 0x03, 0xCC, 0x02, 
	0x00, 0x60, 0xCC, 0x10, 0x07, 0x50, 0x03, 0x4B, 0x02, 0x00, 0xCB, 0x25, 
	0x08, 0x00, 0x00, 0x4C, 0xD8, 0x00, 0x28, 0x00, 0xD2, 0x44, 0x00, 0x00, 
	0x20, 0x87, 0x78, 0x00, 0x72, 0x47, 0xE3, 0x00, 0x20, 0x77, 0x33, 0xE4, 
	0x52, 0x37, 0x38, 0xEE, 0x00, 0x72, 0x83, 0xC8, 0xE4, 0x20, 0x22, 0xCC, 
	0x88, 0x10, 0x00, 0x00, 0xE2, 0x01, 0xFB, 0x3C, 0xED, 0x00, 0x20, 0x02, 
	0xDE, 0xEE, 0x20, 0x22, 0xE2, 0xEA, 0x00, 0xB3, 0xE2, 0xB0, 0x00, 0x03, 
	0x22, 0x10, 0x01, 0x50, 0xFF, 0x77, 0x87, 0x27, 0x00, 0x00, 0x77, 0x33, 
	0x74, 0x02, 0x37, 0xEE, 0x38, 0x27, 0x00, 0xEE, 0xEE, 0x78, 0x23, 0xEE, 
	0xCE, 0x28, 0x02, 0x00, 0xEE, 0x88, 0x2C, 0x00, 0xCC, 0xE8, 0x2E, 0x00, 
	0x40, 0xCE, 0x00, 0xF7, 0xE8, 0xEE, 0x02, 0x00, 0x88, 0xE8, 0x80, 0x01, 
	0x0B, 0x8C, 0x22, 0x02, 0x53, 0xC6, 0x22, 0x22, 0x5B, 0x6C, 0x02, 0x42, 
	0xC7, 0x20, 0xFE, 0xF0, 0xFF, 0x00, 0x30, 0x01, 0x00, 0x15, 0xF7, 0xF0, 
	0xFB, 0xF0, 0xFB, 0x10, 0xFB, 0x00, 0xBB, 0x00, 0xB0, 0x01, 0xF0, 0xFB, 
	0xF0, 0xFB, 0x8C, 0x50, 0xFB, 0x77, 0x47, 0x27, 0x00, 0x46, 0x00, 0xFB, 
	0x20, 0x77, 0x00, 0xE3, 0x00, 0x20, 0x37, 0xE4, 0x00, 0x52, 0x87, 0x00, 
	0xEE, 0x00, 0x72, 0x84, 0xE4, 0x20, 0x75, 0xE8, 0x00, 0x88, 0x20, 0x33, 
	0xC8, 0xEC, 0x20, 0x77, 0xEC, 0x10, 0xEC, 0x00, 0x82, 0x03, 0x03, 0xD2, 
	0xEE, 0xCE, 0x20, 0x40, 0x22, 0x02, 0xC9, 0x22, 0x33, 0x23, 0x22, 0xC2, 
	0x54, 0x0B, 0x2C, 0x22, 0x72, 0x65, 0x03, 0xFF, 0xC2, 0x00, 0x65, 0x00, 
	0xFF, 0x26, 0x83, 0x75, 0x00, 0xFB, 0x3C, 0x02, 0x00, 0xFB, 0x02, 0xEB, 
	0x28, 0x50, 0x00, 0x20, 0xFB, 0xC8, 0x10, 0xFB, 0xCE, 0x02, 0xE8, 0xEE, 
	0x00, 0xC3, 0x02, 0x88, 0xEC, 0x23, 0x00, 0x88, 0x3C, 0x0D, 0x02, 0x00, 
	0x72, 0xC6, 0x00, 0xEF, 0x00, 0xFB, 0x22, 0x70, 0x01, 0xBE, 0xF0, 0xFF, 
	0x00, 0x60, 0x01, 0x00, 0x19, 0xF1, 0x03, 0xF1, 0x03, 0x11, 0x03, 0x00, 
	0xFD, 0xE0, 0x01, 0xF1, 0x03, 0xF1, 0x03, 0x51, 0x03, 0x21, 0x43, 0x21, 
	0x03, 0x37, 0x01, 0x03, 0x20, 0x75, 0xE4, 0x00, 0x03, 0x83, 0x00, 0x82, 
	0x77, 0x85, 0x00, 0x00, 0xD2, 0x38, 0x83, 0x20, 0xEC, 0x78, 0x87, 0x00, 
	0x20, 0xEE, 0x8E, 0xE8, 0x72, 0xE4, 0xEE, 0x3E, 0x03, 0xC2, 0x56, 0xE3, 
	0x2E, 0x22, 0x6C, 0x01, 0xB8, 0xC2, 0xFF, 0x83, 0x11, 0x43, 0x77, 0x83, 
	0x25, 0x00, 0x37, 0x01, 0xE7, 0x00, 0xF2, 0xD0, 0x30, 0x03, 0x61, 0x03, 
	0x2C, 0x01, 0x03, 0xCC, 0x02, 0x88, 0x3E, 0x05, 0x5C, 0x02, 0x22, 0x3C, 
	0x85, 0x00, 0x3B, 0xC6, 0x01, 0x3B, 0xC0, 0x61, 0x04, 0x20, 0xFF, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 
};

struct resource characters_bjin_4bpp = {
	.length = 1536,
	.lz77 = true,
	.data = characters_bjin_4bpp_bytes,
};

static const uint8_t characters_bjin_pal_bytes[] __attribute__((aligned(4))) = {
	0x00, 0x00, 0xFF, 0x7F, 0x00, 0x00, 0x2A, 0x21, 0x2D, 0x1D, 0x32, 0x42, 
	0x55, 0x36, 0x59, 0x3A, 0x66, 0x00, 0xC7, 0x10, 0x09, 0x1D, 0x63, 0x08, 
	0x44, 0x21, 0x4F, 0x7B, 0x24, 0x5E, 0x79, 0x7F, 
};

struct resource characters_bjin_pal = {
	.length = 32,
	.lz77 = false,
	.data = characters_bjin_pal_bytes,
};

