#ifndef MYY_HELPERS_HITBOX_ACTION_H
#define MYY_HELPERS_HITBOX_ACTION_H 1

#include <stdint.h>
#include <myy/helpers/position.h>

struct box_coords_S {
	int16_t left, right, top, bottom;
};

typedef struct box_coords_S box_coords_S_t;

inline static uint8_t box_coords_S_pos_S_inside_window_coords
(box_coords_S_t const box_coords, position_S position)
{
	return 
		(box_coords.left   < position.x &&
		 box_coords.right  > position.x &&
		 box_coords.top    < position.y &&
		 box_coords.bottom > position.y);
}

struct hitbox_action_S {
	box_coords_S_t coords;
	uint8_t (* action)
	(position_S const rel, position_S const abs);
};
typedef struct hitbox_action_S hitbox_action_S_t;

#define HITBOX_ACTION(left, right, top, bottom, action) {\
	.left = left, .right = right, .top = top, .bottom = bottom, \
	.action = action\
}

struct hitboxes_S {
	uint16_t count; uint16_t max;
	hitbox_action_S_t * __restrict const data;
};


typedef struct hitboxes_S hitboxes_S_t;

void hitboxes_action_react_on_click_at
(hitboxes_S_t const * __restrict const hitboxes,
 position_S const abs_screen_click_pos);

hitboxes_S_t hitboxes_struct
(unsigned int const initial_elements_max);

void hitboxes_S_init
(hitboxes_S_t * __restrict const hitboxes,
 unsigned int const initial_elements_max);

uint8_t hitboxes_S_add
(hitboxes_S_t * __restrict const hitboxes,
 int16_t const left, int16_t const right,
 int16_t const top,  int16_t const bottom,
 uint8_t (* action)(position_S rel, position_S abs));

uint8_t hitboxes_S_delete
(hitboxes_S_t * __restrict const hitboxes,
 int16_t const left, int16_t const right,
 int16_t const top,  int16_t const bottom,
 uint8_t (* action)(position_S rel, position_S abs));

uint8_t hitboxes_S_add_copy
(hitboxes_S_t * __restrict const hitboxes,
 hitbox_action_S_t * model);


#endif
