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

__attribute__((unused))
static inline position_S position_S_struct
(int16_t x, int16_t y)
{
	position_S position = { .x = x, .y = y };
	return position;
}

__attribute__((unused))
static inline position_S_3D position_S_3D_struct(
	int16_t const x, int16_t const y, int16_t const z)
{
	position_S_3D position = { .x = x, .y = y };
	return position;
}

__attribute__((unused))
static inline void position_S_set
(position_S * __restrict const position,  int16_t x, int16_t y)
{ 
	position->x = x;
	position->y = y;
}

__attribute__((unused))
static inline position_S position_S_relative_to_window_coords
(position_S const position,
 position_S const other_position)
{
	position_S new_position = {
		/* C++ is too stupid to make a fucking subtraction correctly
		 * It will automatically reconvert every member of the
		 * subtracting to "int" and then cry about narrowing
		 * conversions when trying to store the result back to a type
		 * used by the members of the subtraction initially...
		 * Welcome to Jav^WC#^WC++
		 * Why do all these languages have so much trouble dealing
		 * with 8 and 16 bits numbers...
		 */
		.x = (int16_t) (position.x - other_position.x),
		.y = (int16_t) (position.y - other_position.y)
	};
	return new_position;
}

__attribute__((unused))
static inline position_S * position_S_copy_position
(position_S * __restrict const position, position_S new_position)
{
	position->x = new_position.x;
	position->y = new_position.y;
	return position;
}

__attribute__((unused))
static inline void position_uS_set
(position_uS * __restrict const position, uint16_t x, uint16_t y)
{ position->x = x; position->y = y; }

__attribute__((unused))
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

/* TODO Move away to another header */
struct myy_rectangle {
	int16_t top, bottom, left, right;
};

__attribute__((unused))
static inline position_S position_S_clamp_to_rectangle(
	position_S pos,
	struct myy_rectangle limits)
{
	
	pos.x = (pos.x >= limits.left)  ? pos.x : limits.left;
	pos.x = (pos.x <= limits.right) ? pos.x : limits.right;

	pos.y = (pos.y >= limits.top)    ? pos.y : limits.top;
	pos.y = (pos.y <= limits.bottom) ? pos.y : limits.bottom;

	return pos;
}

#endif
