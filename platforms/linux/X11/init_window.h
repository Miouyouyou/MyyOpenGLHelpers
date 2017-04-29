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

#include <xcb/xcb.h>

#include <myy.h>

struct myy_xcb_state {
	xcb_connection_t * connection;
	/* If too many values like this exist, a structure will be needed */
	unsigned long last_click;
	struct {
		uint32_t button;
		uint16_t start_x, start_y;
	} is_moving;
};

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

	struct myy_xcb_state xcb_state;
};

#ifdef __cplusplus
extern "C" {
#endif

xcb_window_t CreateNativeWindow
(char const * __restrict const title,
 int const width, int const height,
 struct _escontext * __restrict const global_data);
EGLBoolean CreateEGLContext
(char const * __restrict const title,
 int const width, int const height,
 struct _escontext * __restrict const global_data);
EGLBoolean CreateWindowWithEGLContext
(const char * __restrict const title,
 const int width, const int height,
 struct _escontext * __restrict const global_data);
void RefreshWindow
(EGLDisplay const display, EGLSurface const surface);
void Terminate
(Display * display, Window window);

#ifdef __cplusplus
}
#endif

void ParseEvents
(struct myy_xcb_state * __restrict const state);

#endif
