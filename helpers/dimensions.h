#ifndef MYY_HELPERS_DIMENSIONS_H
#define MYY_HELPERS_DIMENSIONS_H 1

#include <stdint.h>

struct myy_uS_dimensions { uint16_t width, height; };
typedef struct myy_uS_dimensions dimensions_uS;

__attribute__((unused))
inline static void dimensions_uS_set
(dimensions_uS * __restrict const dimensions,
 uint16_t width, uint16_t height)
{
	dimensions->width = width; dimensions->height = height;
}

__attribute__((unused))
inline static dimensions_uS dimensions_uS_abs_scale
(dimensions_uS dimensions, uint16_t abs_scale)
{
	dimensions_uS new_dimensions = {
		.width  = (uint16_t) (dimensions.width  + abs_scale),
		.height = (uint16_t) (dimensions.height + abs_scale)
	};
	
	return new_dimensions;
}

struct dimensions_S_ {
	uint16_t width, height;
};

typedef struct dimensions_S_ dimensions_S;

__attribute__((unused))
static inline dimensions_S dimensions_S_struct(
	uint16_t const width, uint16_t const height)
{
	dimensions_S dimensions = { .width = width, .height = height };
	return dimensions;
}


#endif
