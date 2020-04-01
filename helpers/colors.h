#ifndef MYY_HELPERS_COLORS_H
#define MYY_HELPERS_COLORS_H 1

#include <stdint.h>

/**
 * Color with invidual RGBA channels ranging
 * from 0 (minimum) to 255 (maximum).
 */
struct rgba8 {
	uint8_t r, g, b, a;
};

typedef struct rgba8 rgba8_t;

__attribute__((unused))
static inline struct rgba8 rgba8_color(
	uint8_t const r, uint8_t const g, uint8_t const b, uint8_t const a)
{
	struct rgba8 const color = { .r = r, .g = g, .b = b, .a = a };
	return color;
}

/**
 * RGBA8 structure representing a black opaque.
 */
__attribute__((unused))
static inline rgba8_t rgba8_black_opaque(void) {
	rgba8_t const black = {
		.r = 0, .g = 0, .b = 0, .a = 255
	};

	return black;
}

/**
 * RGBA8 structure representing a white opaque.
 */
__attribute__((unused))
static inline rgba8_t rgba8_white_opaque(void) {
	rgba8_t const white = {
		.r = 255, .g = 255, .b = 255, .a = 255
	};
	return white;
}

#endif
