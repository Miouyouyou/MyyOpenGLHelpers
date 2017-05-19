#ifndef MYY_HELPERS_HITBOX_ACTION_H
#define MYY_HELPERS_HITBOX_ACTION_H 1

#include <stdint.h>

struct box_coords_S { int16_t left, right, top, bottom; };

typedef struct box_coords_S box_coords_S_t;

#include <myy/helpers/position.h>



inline static uint8_t box_coords_S_pos_S_inside_window_coords
(box_coords_S_t const box_coords, position_S position)
{
	return 
		(box_coords.left   < position.x &&
		 box_coords.right  > position.x &&
		 box_coords.top    < position.y &&
		 box_coords.bottom > position.y);
}

#define HITBOX_ACTION_SIG void *, position_S const, position_S const
#define HITBOX_ACTION_FULL_SIG(data,rel,abs) \
	void * data, position_S const rel, position_S const abs
struct hitbox_action_S {
	box_coords_S_t coords;
	uint8_t (* action)(HITBOX_ACTION_SIG);
	void * action_data;
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

/**
 * Update the coordinates of a hitbox.
 * 
 * @param hitbox The hitbox to modify the coords from
 * @param new_coords The hitbox new coordinates
 */
inline static void hitbox_action_S_change_coords
(hitbox_action_S_t * __restrict const hitbox,
 box_coords_S_t const new_coords)
{
	hitbox->coords = new_coords;
}

uint8_t hitboxes_action_react_on_click_at
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
 uint8_t (* action)(HITBOX_ACTION_SIG),
 void * action_data);

uint8_t hitboxes_S_delete
(hitboxes_S_t * __restrict const hitboxes,
 int16_t const left, int16_t const right,
 int16_t const top,  int16_t const bottom,
 uint8_t (* action)(HITBOX_ACTION_SIG));

uint8_t hitboxes_S_add_copy
(hitboxes_S_t * __restrict const hitboxes,
 hitbox_action_S_t * model);

uint8_t hitboxes_S_add_box_action
(hitboxes_S_t * __restrict const hitboxes,
 box_coords_S_t * __restrict coords,
 uint8_t (* action)(HITBOX_ACTION_SIG),
 void * action_data);

uint8_t hitboxes_S_delete_box_action
(hitboxes_S_t * __restrict const hitboxes,
 box_coords_S_t * __restrict coords,
 uint8_t (* action)(HITBOX_ACTION_SIG));

/**
 * Reset the hitboxes counter to 0. This does not delete the previously
 * available data, however there is no way to know how many were stored
 * before the reset.
 * 
 * @param hitboxes the hitboxes to reset
 */
inline static void hitboxes_S_quick_reset
(hitboxes_S_t * __restrict const hitboxes)
{
	hitboxes->count = 0;
}

/**
 * Reset the hitboxes counter to 0 and write 0 in all the usable
 * memory.
 * 
 * @param hitboxes the hitboxes to reset
 */
void hitboxes_S_reset
(hitboxes_S_t * __restrict const hitboxes);

#endif
