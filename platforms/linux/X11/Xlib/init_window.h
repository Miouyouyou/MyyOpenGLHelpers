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

#ifndef INIT_WINDOW_INCLUDED
#define INIT_WINDOW_INCLUDED 1

#include <stdint.h>

#include <EGL/egl.h>
#include <EGL/eglplatform.h>

#include <X11/Xlib.h>

#include <myy/myy.h>

/* eglplatform.h implicitly include X11 libraries */
static Atom destroy;

struct _escontext
{
	/// Native System informations
	EGLNativeDisplayType native_display;
	EGLNativeWindowType native_window;
	uint16_t window_width, window_height;
	/// EGL display
	EGLDisplay  display;
	/// EGL context
	EGLContext  context;
	/// EGL surface
	EGLSurface  surface;
};

struct myy_input_state {
	unsigned long last_click_ms;
	struct {
		uint32_t start_button;
		uint16_t start_x, start_y;
	} move_click;
};

struct myy_xlib_state {
	XIM xim;
	XIC xic;
	char * __restrict compose_buffer;
	uintreg_t * running;
};

int CreateNativeWindow(
	struct myy_window_parameters const * __restrict const window_params,
	struct _escontext * __restrict const global_data,
	void * implementation_details);

EGLBoolean CreateEGLContext(
	struct myy_window_parameters const * __restrict const window_params,
	struct _escontext * __restrict const global_data,
	void * implementation_details);

EGLBoolean CreateWindowWithEGLContext(
	struct myy_window_parameters const * __restrict const window_params,
	struct _escontext * __restrict const global_data,
	void * implementation_details);

void ParseEvents(
	myy_states * __restrict const states,
	Display * x_display,
	void * implementation_details);

static inline void RefreshWindow(
	EGLDisplay const display,
	EGLSurface const surface)
{
	eglSwapBuffers(display, surface);
}

void Terminate(
	Display * display,
	Window window,
	void * implementation_details);

#endif
