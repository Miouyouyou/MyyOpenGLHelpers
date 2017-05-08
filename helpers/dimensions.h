#ifndef MYY_HELPERS_DIMENSIONS_H
#define MYY_HELPERS_DIMENSIONS_H 1

#include <stdint.h>

struct myy_uS_dimensions { uint16_t width, height; };
typedef struct myy_uS_dimensions dimensions_uS;

inline static void dimensions_uS_set
(dimensions_uS * __restrict const dimensions,
 uint16_t width, uint16_t height)
{
	dimensions->width = width; dimensions->height = height;
}
inline static dimensions_uS dimensions_uS_abs_scale
(dimensions_uS dimensions, uint16_t abs_scale)
{
	dimensions_uS new_dimensions = {
		.width  = dimensions.width + abs_scale,
		.height = dimensions.height + abs_scale
	};
	
	return new_dimensions;
}

#endif
