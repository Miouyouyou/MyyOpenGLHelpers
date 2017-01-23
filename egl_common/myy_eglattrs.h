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

#ifndef _MYY_EGL_ATTRS_H_
#define _MYY_EGL_ATTRS_H_ 1

#include <EGL/egl.h>

#ifdef EGL_OPENGL_ES3_BIT
#define MYY_GLES3_BIT EGL_OPENGL_ES3_BIT
#else
#include <EGL/eglext.h>
#define MYY_GLES3_BIT EGL_OPENGL_ES3_BIT_KHR
#endif

#define MYY_EGL_COMMON_PC_ATTRIBS \
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,   \
	EGL_CONFORMANT, MYY_GLES3_BIT, \
	EGL_SAMPLES,         4, \
	EGL_RED_SIZE,        5, \
	EGL_GREEN_SIZE,      6, \
	EGL_BLUE_SIZE,       5, \
	EGL_ALPHA_SIZE,      8, \
	EGL_DEPTH_SIZE,     16

#define MYY_EGL_COMMON_MOBILE_ATTRIBS MYY_EGL_COMMON_PC_ATTRIBS

#define MYY_CURRENT_GL_CONTEXT EGL_CONTEXT_CLIENT_VERSION, 3

#endif
