/*
  Copyright (c) 2017 Miouyouyou <Myy>

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

#ifndef MYY_CURRENT_OPENGL
#define MYY_CURRENT_OPENGL 1

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl31.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifdef EGL_OPENGL_ES3_BIT
#define MYY_GLES3_BIT EGL_OPENGL_ES3_BIT
#else
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

#define MYY_CURRENT_GL_CONTEXT \
	EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 1, EGL_NONE

#endif
