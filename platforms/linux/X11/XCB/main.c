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
#include <locale.h>
#include "init_window.h"

static void stop(unsigned int * running) {
	*running = 0;
}

int main(int argc, char **argv) {

	struct _escontext global_context;
	struct myy_window_parameters window_params = {0};

	int ret = myy_init(argc, argv, &window_params);
	if (ret)
		exit(ret);

	window_params.title =
		(window_params.title != NULL)
		? window_params.title : "Myy Window";
	window_params.width =
		(window_params.width != 0)
		? window_params.width : 1280;
	window_params.height =
		(window_params.height != 0)
		? window_params.height : 720;

	uint8_t context_created = CreateWindowWithEGLContext(
		&window_params, &global_context
	);
	if (!context_created) exit(1);

	myy_generate_new_state();
	myy_init_drawing();
	myy_display_initialised(
		global_context.window_width,
		global_context.window_height);

	unsigned int running = 1;

	struct myy_platform_handlers * handlers =
		myy_get_platform_handlers();

	handlers->stop = stop;
	handlers->stop_data = &running;

	while(running) {
		ParseEvents(global_context.connection);
		myy_draw();
		myy_after_draw();
		RefreshWindow(
			global_context.display, global_context.surface
		);
	}

	myy_stop();
	myy_cleanup_drawing();
	Terminate(
		global_context.native_display,
		global_context.native_window
	);

}
