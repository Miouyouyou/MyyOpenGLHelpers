#ifndef MYY_HELPERS_POSITION_H
#define MYY_HELPERS_POSITION_H 1

#include <stdint.h>

struct myy_S_position  {  int16_t x, y; };
struct myy_uS_position { uint16_t x, y; };
struct myy_I_position  {  int32_t x, y; };

typedef struct myy_S_position  position_S;
typedef struct myy_uS_position position_uS;
typedef struct myy_I_position  position_I;

#include <myy/helpers/hitbox_action.h>

static inline position_S position_S_struct
(uint16_t x, uint16_t y)
{
	position_S position = { .x = x, .y = y };
	return position;
}
static inline void position_S_set
(position_S * __restrict const position,  int16_t x, int16_t y)
{ position->x = x; position->y = y; }

static inline position_S position_S_box_coords_S_top_left
(struct box_coords_S box_coords)
{
	return position_S_struct(box_coords.left, box_coords.top);
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

static inline position_S position_S_copy_position
(position_S * __restrict const position, position_S new_position)
{
	position->x = new_position.x;
	position->y = new_position.y;
}

static inline void position_uS_set
(position_uS * __restrict const position, uint16_t x, uint16_t y)
{ position->x = x; position->y = y; }

static inline void position_I_set
(position_I * __restrict const position,  int32_t x, int32_t y)
{ position->x = x; position->y = y; }

#endif
