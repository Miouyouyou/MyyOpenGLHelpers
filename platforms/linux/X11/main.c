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

#include <myy.h>
#include <myy/helpers/log.h>
#include <myy/helpers/strings.h>
#include <stdlib.h>
#include <string.h>
#include "init_window.h"

static void stop(unsigned int * running) {
	*running = 0;
}

int main() {

	if (!CreateWindowWithEGLContext("Nya !", 1280, 720)) exit(1);
	myy_generate_new_state();
	myy_init_drawing();
	myy_display_initialised(1280, 720);

	unsigned int running = 1;
	struct myy_platform_handlers * handlers =
		myy_get_platform_handlers();
	
	handlers->stop = stop;
	handlers->stop_data = &running;
	
	while(running) {
		ParseEvents();
		myy_draw();
		myy_after_draw();
		RefreshWindow();
	}

	myy_stop();
	myy_cleanup_drawing();

}
