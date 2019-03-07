#ifndef MYY_HELPERS_POSITION_H
#define MYY_HELPERS_POSITION_H 1

#include <stdint.h>

struct myy_S_position    {  int16_t x, y; };
struct myy_uS_position   { uint16_t x, y; };
struct myy_I_position    {  int32_t x, y; };
struct myy_S_position_3D {  int16_t x, y, z; };
struct myy_S_position_4D {  int16_t x, y, z, w; };

typedef struct myy_S_position  position_S;
typedef struct myy_uS_position position_uS;
typedef struct myy_I_position  position_I;
typedef struct myy_S_position_3D position_S_3D;
typedef struct myy_S_position_4D position_S_4D;

static inline position_S position_S_struct
(int16_t x, int16_t y)
{
	position_S position = { .x = x, .y = y };
	return position;
}

static inline position_S_3D position_S_3D_struct(
	int16_t const x, int16_t const y, int16_t const z)
{
	position_S_3D position = { .x = x, .y = y };
	return position;
}

static inline void position_S_set
(position_S * __restrict const position,  int16_t x, int16_t y)
{ 
	position->x = x;
	position->y = y;
}

static inline position_S position_S_relative_to_window_coords
(position_S const position,
 position_S const other_position)
{
	position_S new_position = {
		.x = position.x - other_position.x,
		.y = position.y - other_position.y
	};
	return new_position;
}

static inline position_S * position_S_copy_position
(position_S * __restrict const position, position_S new_position)
{
	position->x = new_position.x;
	position->y = new_position.y;
	return position;
}

static inline void position_uS_set
(position_uS * __restrict const position, uint16_t x, uint16_t y)
{ position->x = x; position->y = y; }

static inline void position_I_set
(position_I * __restrict const position,  int32_t x, int32_t y)
{ position->x = x; position->y = y; }

#define POSITION_4D_SIMPLE(x, y, z) {x,y,z,1}

__attribute__((unused))
static inline position_S position_S_2D_from_4D(position_S_4D pos)
{
	position_S pos_2D = {
		.x = pos.x,
		.y = pos.y
	};

	return pos_2D;
}

__attribute__((unused))
static inline position_S_4D position_S_4D_struct(
	int16_t const x,
	int16_t const y,
	int16_t const z,
	int16_t const w)
{
	position_S_4D pos_4D = { .x = x, .y = y, .z = z, .w = w };
	return pos_4D;
}


#endif
