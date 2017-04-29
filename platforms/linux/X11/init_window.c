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
#include "init_window.h"

#define TRUE 1
#define FALSE 0

#include <string.h>
#include <stdlib.h>

#include <myy/current/opengl.h>
#include <myy/helpers/strings.h>
#include <myy/helpers/log.h>

#include <xcb/xcb.h>
#include <xcb/xkb.h>

enum myy_eglstatus {
	myy_eglstatus_no_problem,
	myy_eglstatus_eglGetDisplay,
	myy_eglstatus_eglInitialize,
	myy_eglstatus_eglGetConfigs,
	myy_eglstatus_eglChooseConfig,
	myy_eglstatus_eglBindAPI,
	myy_eglstatus_eglCreateWindowSurface,
	myy_eglstatus_eglCreateContext,
	myy_eglstatus_eglMakeCurrent,
	myy_eglstatus_egl_getvisualid,
	myy_eglstatus_n
};

static char const * const myy_eglerrors[myy_eglstatus_n] = {
	[myy_eglstatus_no_problem] = "Everything OK with EGL !",
	[myy_eglstatus_eglGetDisplay] = "Could not GetDisplay",
	[myy_eglstatus_eglInitialize] = "Could not initialize EGL",
	[myy_eglstatus_eglGetConfigs] =
		"Could not retrieve EGL configurations",
	[myy_eglstatus_eglChooseConfig] =
		"Could not choose an EGL configuration",
	[myy_eglstatus_eglBindAPI] = "Could not bind the API",
	[myy_eglstatus_eglCreateWindowSurface] =
		"Could not create an EGL Window surface",
	[myy_eglstatus_eglCreateContext] =
		"Could not create an EGL Context",
	[myy_eglstatus_eglMakeCurrent] =
		"Could not make the current EGL context current",
	[myy_eglstatus_egl_getvisualid] =
		"Could not get the visual id of the chosen configuration"
};

xcb_window_t CreateNativeWindow
(char const * __restrict const title,
 int const width, int const height,
 struct _escontext * __restrict const global_data)
{
	Display * display = XOpenDisplay(NULL);
	xcb_connection_t * const connection = xcb_connect(NULL, NULL);
	xcb_screen_t * const screen =
		xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
	
	xcb_window_t window = xcb_generate_id(connection);
	
	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t values[2] = {
		screen->white_pixel,
		XCB_EVENT_MASK_EXPOSURE         |
		XCB_EVENT_MASK_BUTTON_PRESS     |
		XCB_EVENT_MASK_BUTTON_RELEASE   |
		XCB_EVENT_MASK_POINTER_MOTION   |
		XCB_EVENT_MASK_ENTER_WINDOW     |
		XCB_EVENT_MASK_LEAVE_WINDOW     |
		XCB_EVENT_MASK_KEY_PRESS        |
		XCB_EVENT_MASK_KEY_RELEASE      |
		XCB_EVENT_MASK_RESIZE_REDIRECT  |
		XCB_EVENT_MASK_STRUCTURE_NOTIFY
	};
	
	xcb_void_cookie_t create_cookie = xcb_create_window_checked(
		connection,
		XCB_COPY_FROM_PARENT,
		window,
		screen->root,
		0, 0,
		width, height,
		0,
		XCB_WINDOW_CLASS_INPUT_OUTPUT,
		screen->root_visual,
		mask, values
	);
	xcb_change_property(
		connection,
		XCB_PROP_MODE_REPLACE,
		window,
		XCB_ATOM_WM_NAME,
		XCB_ATOM_STRING,
		8,
		strlen (title),
		title
	);
	xcb_void_cookie_t map_cookie = 
		xcb_map_window_checked(connection, window);
	
	xcb_generic_error_t * error =
		xcb_request_check(connection, create_cookie);
	if (error) LOG("Could not create a window !?\n");
	error = xcb_request_check(connection, map_cookie);
	if (error) LOG("Could not map a window !?\n");
	
	xcb_flush(connection);

	xcb_intern_atom_cookie_t wmDeleteCookie = xcb_intern_atom(
		connection, 0, strlen("WM_DELETE_WINDOW"),
		"WM_DELETE_WINDOW"
	);
	xcb_intern_atom_cookie_t wmProtocolsCookie = xcb_intern_atom(
		connection, 0, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS"
	);
	xcb_intern_atom_reply_t *wmDeleteReply =
		xcb_intern_atom_reply(connection, wmDeleteCookie, NULL);
	xcb_intern_atom_reply_t *wmProtocolsReply =
		xcb_intern_atom_reply(connection, wmProtocolsCookie, NULL);
		
	xcb_change_property(
		connection, XCB_PROP_MODE_REPLACE, window,
		wmProtocolsReply->atom, 4, 32, 1, &wmDeleteReply->atom
	);
	
	/* Predictable autorepeat is essential for correct input management
	 * When a key is hold down :
	 * - Without predictable autorepeat, the application will receive :
	 *   "key_pressed", "key_released", "key_pressed", "key_released"
	 * - With predictable autorepeat, the application will receive :
	 *   "key_pressed, "key_pressed", ..., "key_released"
	 * Which helps differentiate between autorepeat and the same key
	 * actually pressed multiple times.
	 */
	xcb_xkb_use_extension(connection, 1, 0);
	xcb_xkb_per_client_flags_cookie_t repeat = xcb_xkb_per_client_flags(
		connection,
		XCB_XKB_ID_USE_CORE_KBD,
		XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
		XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
		0,0,0
	);

  global_data->native_display = display;
  global_data->window_width   = width;
  global_data->window_height  = height;
  global_data->native_window  = window;
	
	global_data->xcb_state.connection = connection;
	
	return window;
}

EGLBoolean CreateEGLContext
(char const * __restrict const title,
 int const width, int const height,
 struct _escontext * __restrict const global_data)
{
	enum myy_eglstatus current_status = myy_eglstatus_no_problem;
	EGLBoolean return_status = EGL_FALSE;
	EGLint numConfigs;
	EGLint majorVersion;
	EGLint minorVersion;
	EGLContext context;
	EGLSurface surface;
	EGLConfig config;
	EGLint eglConfAttrVisualID;
	EGLint eglAttribs[] =  {
		MYY_EGL_COMMON_PC_ATTRIBS,
		EGL_NONE, EGL_NONE
	};
	/* The system can clearly provide you a OpenGL ES 2.x compliant
	   configuration without OpenGL ES 2.x enabled ! */
	EGLint contextAttribs[] =
		{ MYY_CURRENT_GL_CONTEXT, EGL_NONE, EGL_NONE };

	xcb_window_t native_window =
		CreateNativeWindow(title, width, height, global_data);
	EGLDisplay display = 
		eglGetDisplay( global_data->native_display );
	if ( display == EGL_NO_DISPLAY ) {
		current_status = myy_eglstatus_eglGetDisplay;
		goto print_current_egl_status_and_return;
	}

	// Initialize EGL
	if ( !eglInitialize(display, &majorVersion, &minorVersion) ) {
		current_status = myy_eglstatus_eglInitialize;
		goto print_current_egl_status_and_return;
	}

	// Get configs
	if (   (eglGetConfigs(display, NULL, 0, &numConfigs) != EGL_TRUE)
			|| (numConfigs == 0)) {
		current_status = myy_eglstatus_eglGetConfigs;
		goto print_current_egl_status_and_return;
	}

	// Choose config
	if ( (eglChooseConfig(display, eglAttribs, &config, 1, &numConfigs)
		  != EGL_TRUE)
			|| (numConfigs != 1)) {
		current_status = myy_eglstatus_eglChooseConfig;
		goto print_current_egl_status_and_return;
	}

	if (eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE) {
		current_status = myy_eglstatus_eglBindAPI;
		goto print_current_egl_status_and_return;
	}
	
	// Create a GL context
	context = eglCreateContext(
		display, config, EGL_NO_CONTEXT, contextAttribs
	);

	if ( context == EGL_NO_CONTEXT ) {
		current_status = myy_eglstatus_eglCreateContext;
		goto print_current_egl_status_and_return;
	}
	

	// Create a surface
	surface = eglCreatePlatformWindowSurface(
		display, config, &native_window, NULL
	);

	if ( surface == EGL_NO_SURFACE ) {
		current_status = myy_eglstatus_eglCreateWindowSurface;
		goto print_current_egl_status_and_return;
	}

	// Make the context current
	if ( !eglMakeCurrent(display, surface, surface, context) ) {
		current_status = myy_eglstatus_eglMakeCurrent;
		goto print_current_egl_status_and_return;
	}

	global_data->display = display;
	global_data->surface = surface;
	global_data->context = context;
	return_status = EGL_TRUE;

print_current_egl_status_and_return:
	LOG(
		"%s - Error code (%04x)\n",
		myy_eglerrors[current_status], eglGetError()
	);
	return return_status;
}

void RefreshWindow
(EGLDisplay const display, EGLSurface const surface)
{
	eglSwapBuffers(display, surface);
}

EGLBoolean CreateWindowWithEGLContext
(const char * __restrict const title,
 const int width, const int height,
 struct _escontext * __restrict const global_data)
{
  return CreateEGLContext(title, width, height, global_data);
}



void Terminate
(Display * display, Window window)
{
  if (window) XDestroyWindow( display, window );
  XCloseDisplay( display );
}

