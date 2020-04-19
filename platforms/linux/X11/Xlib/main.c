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

#include <myy/myy.h>
#include <myy/helpers/log.h>
#include <myy/helpers/strings.h>
#include <stdlib.h>
#include <string.h>
#include "init_window.h"

void myy_platform_stop(myy_states * __restrict const states)
{
	struct myy_xlib_state * __restrict const platform_state =
		(myy_xlib_state * __restrict) (states->platform_state);
	*platform_state->running = 0;
}

static uint64_t elapsed_time_ns(
	struct timespec * __restrict const ts)
{
	clock_gettime(CLOCK_MONOTONIC_RAW, ts);
	return ts->tv_sec * 1000000000 + ts->tv_nsec;
}

int main(int argc, char **argv) {

	uintreg_t running = 1;
	struct _escontext global_context;
	struct myy_window_parameters window_params = {0};
	struct myy_xlib_state implementation_details = {0};
	implementation_details.running = &running;

	myy_states states = {0};
	states.platform_state = &implementation_details;
	int ret = myy_init(&states, argc, argv, &window_params);
	if (ret)
		exit(ret);

	LOG("Using XLIB\n");
	window_params.title =
		(window_params.title != NULL)
		? window_params.title : "Myy Window";
	window_params.width =
		(window_params.width != 0)
		? window_params.width : 1920;
	window_params.height =
		(window_params.height != 0)
		? window_params.height : 1080;

	uint8_t context_created = CreateWindowWithEGLContext(
		&window_params, &global_context, &implementation_details);
	if (!context_created) exit(1);

	myy_init_drawing(&states,
		global_context.window_width, global_context.window_height);
	myy_display_initialised(
		&states,
		global_context.window_width,
		global_context.window_height);

	uint64_t last_frame_ns;
		
	uint64_t current_frame_ns = 
		elapsed_time_ns(&states.current_frame_time);
	uint64_t delta_ns;
	uint64_t i = 0;
	
	while(running) {
		/* Get the current times */
		last_frame_ns =
			current_frame_ns;
		current_frame_ns = elapsed_time_ns(&states.current_frame_time);
		delta_ns =
			current_frame_ns - last_frame_ns;

		/* TODO Do we need THAT much arguments ? */
		ParseEvents(
			&states,
			global_context.native_display,
			&implementation_details);
		myy_draw_before(&states, i, delta_ns);
		myy_draw(&states, i, delta_ns);
		myy_draw_after(&states, i, delta_ns);
		RefreshWindow(
			global_context.display, global_context.surface
		);
		i++;
	}

	myy_stop(&states);
	myy_cleanup_drawing(&states);
	Terminate(
		global_context.native_display,
		global_context.native_window,
		&implementation_details
	);

}
