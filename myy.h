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

#include <myy/helpers/fonts/packed_fonts_display.h>
#include <myy/helpers/hitbox_action.h>

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

void myy_click(
	myy_states * __restrict state,
	int x,
	int y,
	unsigned int button);
void myy_doubleclick(
	myy_states * __restrict state,
	int x,
	int y,
	unsigned int button);
void myy_hover(
	myy_states * __restrict state,
	int x,
	int y);
void myy_move(
	myy_states * __restrict state,
	int x, int y,
	int start_x, int start_y);
void myy_key(
	myy_states * __restrict state,
	unsigned int keycode);
void myy_key_release(
	myy_states * __restrict state,
	unsigned int keycode);
void myy_text(
	myy_states * __restrict state,
	char const * __restrict const text,
	size_t const text_size);

/* Temporary changes */
enum mouse_action_type { myy_mouse_wheel_action };

void myy_rel_mouse_move(myy_states * __restrict state, int x, int y);
void myy_mouse_action(
	myy_states * __restrict state,
	enum mouse_action_type,
	int value);

void myy_text_input_start(myy_states * __restrict state);
void myy_text_input_stop(myy_states * __restrict state);

void myy_platform_stop(myy_states * __restrict state);

#endif 
