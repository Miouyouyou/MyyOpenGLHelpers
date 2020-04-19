/*
  Copyright (c) 2016 Miouyouyou <Myy>

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files 
  (the "Software"), to deal in the Software without restriction, 
  including without limitation the rights to use, copy, modify, merge, 
  publish, distribute, sublicense, and/or sell copies of the Software, 
  and to permit persons to whom the Software is furnished to do so, 
  subject to the following conditions:

  The above copyright notice and this permission notice shall be 
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MYY_INCLUDED
#define MYY_INCLUDED 1

#include "helpers/c_types.h"
#include "helpers/strings.h"
#include "current/opengl.h"

#include "helpers/arrays.h"
#include "helpers/buffers.h"
#include "helpers/colors.h"
#include "helpers/dimensions.h"
#include "helpers/file.h"
#include "helpers/hitbox_action.h"
#include "helpers/log.h"
#include "helpers/macros.h"
#include "helpers/matrices.h"
#include "helpers/memory.h"
#include "helpers/myy_memory.h"
#include "helpers/opengl/buffers.h"
#include "helpers/opengl/loaders.h"
#include "helpers/opengl/shaders_pack.h"
#include "helpers/position.h"
#include "helpers/random.h"
#include "helpers/struct.h"
#include "helpers/temp_string.h"
#include "helpers/vector.h"

#ifdef __cplusplus
#include "helpers/myy_vector.hpp"
#include "helpers/points.hpp"

#endif

#ifndef _POSIX_C_SOURCE
#define HAD_TO_DEFINE_POSIX_C_SOURCE 1
#define _POSIX_C_SOURCE 201901L
#endif

#include <stdint.h>
#include <time.h>

#ifdef HAD_TO_DEFINE_POSIX_C_SOURCE
#undef HAD_TO_DEFINE_POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

enum myy_input_events {
	myy_input_event_invalid,
	myy_input_event_mouse_moved_absolute,
	myy_input_event_mouse_moved_relative,
	myy_input_event_mouse_button_pressed,
	myy_input_event_mouse_button_released,
	myy_input_event_touch_pressed,
	myy_input_event_touch_move,
	myy_input_event_touch_released,
	myy_input_event_keyboard_key_pressed,
	myy_input_event_keyboard_key_released,
	myy_input_event_text_received,
	myy_input_event_editor_finished,
	myy_input_event_surface_size_changed,
	myy_input_event_window_destroyed,
	myy_input_event_window_focus_in,
	myy_input_event_window_focus_out,
	myy_input_event_android_state,
};

enum myy_android_state {
	myy_android_state_invalid,
	myy_android_state_start,
	myy_android_state_resume,
	myy_android_state_pause,
	myy_android_state_stop,
	myy_android_state_force_load,
	myy_android_state_force_save,
};

union myy_input_event_data {
	struct {
		int16_t x, y;
		uint8_t index; // For multi peripherals event
		uint8_t type;  // 0: relative, 1: absolute
	} mouse_move_relative;
	struct {
		int16_t x, y;
		uint8_t index;
		uint8_t type;
	} mouse_move_absolute;
	struct {
		/* Turns out that you can release the mouse outside the window,
		 * in which case x and/or y could be negative.
		 */
		int16_t x, y;
		uint8_t index;
		// TODO That generates an useless write, it seems.
		uint8_t state; // 0: pressed, 1: released
		uint8_t button_number;
		uint8_t unused[3]; // align on 4 bytes
	} mouse_button;
	struct {
		uint16_t x, y;
		uint8_t state; // 0: pressed, 1: released, 3: released and canceled
		uint8_t move;  // 0: Just a press, 1: Move action
		int32_t id;

	} touch;
	struct {
		uint32_t raw_code;
		uint32_t modifiers;
		uint8_t state;
		uint8_t unused[7]; // align on 8 bytes
	} key;
	struct {
		char const * __restrict data;
		size_t length;
	} text;
	struct {
		uint16_t width, height;
		uint8_t unused[4];
	} surface;
	struct {
		uint8_t unused[8];
	} window_destroyed;
	struct {
		uint8_t unused[8];
	} window_focus;
	struct {
		enum myy_android_state state;
	} android;
};

struct myy_states_s {
	void * user_state;
	uint32_t surface_width, surface_height;
	void * platform_state;
	void * game_state;
	// TODO Useless ?
	void * platform_handlers;
	struct timespec current_frame_time;
};
typedef struct myy_states_s myy_states;

#include <myy/helpers/fonts/packed_fonts_display.h>
#include <myy/helpers/hitbox_action.h>

#include <myy/helpers/strings.h>

struct myy_common_data {
	struct glyph_infos * __restrict fonts_glyphs;
	hitboxes_S_t * hitboxes;
};

struct myy_window_parameters {
	char const * __restrict title;
	uintreg_t width;
	uintreg_t height;
};

void myy_display_initialised(
	myy_states * __restrict state,
	unsigned int width,
	unsigned int height);
int myy_init(
	myy_states * __restrict state,
	int argc,
	char **argv,
	struct myy_window_parameters * __restrict const parameters);
void myy_init_drawing(
	myy_states * __restrict state,
	uintreg_t surface_width,
	uintreg_t surface_height);
/* TODO Can be an issue wiht ARMv7.
 * How about :
 * last_frame_delta_s  (32 bits),
 * last_frame_delat_ns (32 bits)
 */
void myy_draw_before(
	myy_states * __restrict state,
	uintreg_t i,
	uint64_t last_frame_delta_ns);
void myy_draw(
	myy_states * __restrict state, 
	uintreg_t i,
	uint64_t last_frame_delta_ns);
void myy_draw_after(
	myy_states * __restrict state,
	uintreg_t i,
	uint64_t last_frame_delta_ns);
void myy_cleanup_drawing(
	myy_states * __restrict state);
void myy_stop(
	myy_states * __restrict state);
/* User quit is a "Quit" action performed by the user,
 * using the program UI.
 * This is different from closing the window abruptly. */
void myy_user_quit(myy_states * __restrict state);

void myy_save_state(
	myy_states * __restrict state,
	void * game_state);
void myy_resume_state(
	myy_states * __restrict state,
	void * game_state);

void myy_input(
	myy_states * __restrict state,
	enum myy_input_events const event_type,
	union myy_input_event_data * __restrict const event);

void myy_text_input_start(myy_states * __restrict state);
void myy_text_input_stop(myy_states * __restrict state);

void myy_platform_stop(myy_states * __restrict state);

#endif 
