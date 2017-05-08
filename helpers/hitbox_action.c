#include <myy/helpers/arrays.h>
#include <myy/helpers/hitbox_action.h>
#include <myy/helpers/memory.h>

#include <string.h> // memcmp

/*
 * void hitboxes_action_react_on_click_at
(hitboxes_S_t const * __restrict const hitboxes,
 position_S const clicked_pos_rel_to_screen)
 */
void hitboxes_action_react_on_click_at
(hitboxes_S_t const * __restrict const hitboxes,
 position_S const clicked_pos_rel_to_screen)
{
	uint_fast16_t n_hitboxes = hitboxes->count;
	for (uint_fast16_t i = 0; i < n_hitboxes; i++)
	{
		struct hitbox_action_S current_hitbox = hitboxes->data[i];
		box_coords_S_t box_coords = current_hitbox.coords;
		
		uint8_t clicked_inside = box_coords_S_pos_S_inside_window_coords(
			box_coords, clicked_pos_rel_to_screen
		);
		
		if (clicked_inside) {
			
			position_S box_top_left = 
				position_S_struct(box_coords.left, box_coords.top);
			
			position_S clicked_pos_rel_to_box =
				position_S_relative_to_window_coords(
					box_top_left, clicked_pos_rel_to_screen
				);
			
			uint_fast8_t click_handled = current_hitbox.action(
				clicked_pos_rel_to_box, clicked_pos_rel_to_screen
			);
			if (click_handled) break;
		}
	}
	
}

hitboxes_S_t hitboxes_struct
(unsigned int const initial_elements_max)
{
	hitboxes_S_t const hitboxes = {
		.count = 0, .max = initial_elements_max,
		.data = (hitbox_action_S_t *) allocate_durable_memory(
			initial_elements_max * sizeof(hitbox_action_S_t)
		)
	};
	return hitboxes;
}

void hitboxes_S_init
(hitboxes_S_t * __restrict const hitboxes,
 unsigned int const initial_elements_max)
{
	hitboxes_S_t const hb_struct = hitboxes_struct(initial_elements_max);
	memcpy(hitboxes, &hb_struct, sizeof(hitboxes_S_t));
}

inline static hitbox_action_S_t hitbox_action_S_struct
(int16_t const left, int16_t const right,
 int16_t const top, int16_t const bottom,
 uint8_t (* action)(position_S rel, position_S abs))
{
	hitbox_action_S_t hitbox = {
		.coords = {
			.left = left, .right = right, .top = top, .bottom = bottom
		},
		.action = action
	};
	return hitbox;
}

uint8_t hitboxes_S_add
(hitboxes_S_t * __restrict const hitboxes,
 int16_t const left, int16_t const right,
 int16_t const top,  int16_t const bottom,
 uint8_t (* action)(position_S rel, position_S abs))
{
	uint8_t enough_space = dyn_array_generic_ensure_space_for_more_u16(
		(DynArray_u16_t * __restrict) hitboxes, 1, sizeof(DynArray_u16_t)
	);
	
	if (enough_space) {
		hitbox_action_S_t hitbox = hitbox_action_S_struct(
			left, right, top, bottom, action
		);

		hitboxes->data[hitboxes->count] = hitbox;
		hitboxes->count += 1;
	}
	return enough_space;
}

uint8_t hitboxes_S_add_copy
(hitboxes_S_t * __restrict const hitboxes,
 hitbox_action_S_t * model)
{
	uint8_t enough_space = dyn_array_generic_ensure_space_for_more_u16(
		(DynArray_u16_t * __restrict) hitboxes, 1, sizeof(DynArray_u16_t)
	);
	if (enough_space) {
		hitboxes->data[hitboxes->count] = *model;
		hitboxes->count += 1;
	}
	
	return enough_space;
}

uint8_t hitboxes_S_delete
(hitboxes_S_t * __restrict const hitboxes,
 int16_t const left, int16_t const right,
 int16_t const top,  int16_t const bottom,
 uint8_t (* action)(position_S rel, position_S abs))
{
	hitbox_action_S_t hitbox = hitbox_action_S_struct(
		left, right, top, bottom, action
	);
	uint_fast16_t n_hitboxes = hitboxes->count;
	uint_fast16_t h = 0;
	while(h < n_hitboxes) {
		int identical = (
			0 ==
			memcmp(hitboxes->data+h, &hitbox, sizeof(hitbox_action_S_t))
		);
		if (!identical) h++;
		else {
			uint_fast16_t next_h = h + 1;
			uint_fast16_t remaining_hitboxes = n_hitboxes - h - 1;
			recopy_inside_memory_space(
				hitboxes->data+h,
				hitboxes->data+next_h,
				remaining_hitboxes * sizeof(hitbox)
			);
			hitboxes->count--;
			break;
		}
	}
	return h < n_hitboxes;
}


