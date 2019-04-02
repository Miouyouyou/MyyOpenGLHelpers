/*
  Copyright (c) 2019 Miouyouyou <Myy>

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
	[myy_eglstatus_no_problem] =
		"Everything OK with EGL !",
	[myy_eglstatus_eglGetDisplay] =
		"Could not GetDisplay",
	[myy_eglstatus_eglInitialize] =
		"Could not initialize EGL",
	[myy_eglstatus_eglGetConfigs] =
		"Could not retrieve EGL configurations",
	[myy_eglstatus_eglChooseConfig] =
		"Could not choose an EGL configuration",
	[myy_eglstatus_eglBindAPI] =
		"Could not bind the API",
	[myy_eglstatus_eglCreateWindowSurface] =
		"Could not create an EGL Window surface",
	[myy_eglstatus_eglCreateContext] =
		"Could not create an EGL Context",
	[myy_eglstatus_eglMakeCurrent] =
		"Could not make the current EGL context current",
	[myy_eglstatus_egl_getvisualid] =
		"Could not get the visual id of the chosen configuration"
};

#define TRUE 1
#define FALSE 0

#include <stdlib.h>
int CreateNativeWindow
(struct myy_window_parameters const * __restrict const window_params,
 struct _escontext * __restrict const global_data)
{
	int ret = 0;
	int const width = window_params->width;
	int const height = window_params->height;
	/* Subject to various security flaws :
	 * 1. There's no guarantee that the string is NULL terminated.
	 * 2. There's no way to guarantee this NULL termination.
	 * TODO Use a specify string structure that guarantees these two
	 * things.
	 */
	char const * __restrict const title = window_params->title;
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
	xcb_void_cookie_t map_cookie;
	xcb_generic_error_t * error =
		xcb_request_check(connection, create_cookie);

	if (error) {
		LOG("xcb_create_window failed : Could not create a window !?\n");
		ret = -ENOSYS;
		goto error;
	}

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
	map_cookie = xcb_map_window_checked(connection, window);

	error = xcb_request_check(connection, map_cookie);
	if (error) {
		LOG("xcb_map_window failed : Could not map the window !?\n");
		/* FIXME What about the window we just created ? */
		ret = -ENOSYS;
		goto error;
	}
	
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
	global_data->connection     = connection;

	return ret;

error:
	return ret;
}

EGLBoolean CreateEGLContext
(struct myy_window_parameters const * __restrict const parameters,
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
	EGLint eglAttribs[] =  {
		MYY_EGL_COMMON_PC_ATTRIBS,
		EGL_NONE, EGL_NONE
	};
	/* The system can clearly provide you a OpenGL ES 2.x compliant
	   configuration without OpenGL ES 2.x enabled ! */
	EGLint contextAttribs[] =
		{ MYY_CURRENT_GL_CONTEXT, EGL_NONE, EGL_NONE };

	char const * __restrict const supported_extensions =
        eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	EGLDisplay display = EGL_NO_DISPLAY;

	/* Let's use eglGetPlatformDisplay if possible */
	if (supported_extensions
		&& strstr(supported_extensions, "EGL_EXT_platform_base"))
	{
		LOG("Trying to use eglGetPlatformDisplayEXT()\n");

		/* Got to love these short macro names */
		PFNEGLGETPLATFORMDISPLAYEXTPROC get_platform_display =
			(PFNEGLGETPLATFORMDISPLAYEXTPROC)
			eglGetProcAddress("eglGetPlatformDisplayEXT");

		if( get_platform_display != NULL ) {
			display = get_platform_display(
				EGL_PLATFORM_X11_KHR,
				global_data->native_display,
				NULL);
			if (display != EGL_NO_DISPLAY)
				LOG("Success !\n");
			else
				LOG("Failed with error %04x\n", eglGetError());
		}
		else
			LOG("No eglGetPlatformDisplay extension... Seriously ?\n");
	}

	if (display == EGL_NO_DISPLAY) {
		LOG("Using the old eglGetDisplay call.\n");
		display = eglGetDisplay( global_data->native_display );
	}

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
	if ((eglGetConfigs(display, NULL, 0, &numConfigs) != EGL_TRUE)
	    || (numConfigs == 0))
	{
		current_status = myy_eglstatus_eglGetConfigs;
		goto print_current_egl_status_and_return;
	}

	// Choose config
	if ((eglChooseConfig(display, eglAttribs, &config, 1, &numConfigs)
	    != EGL_TRUE)
	    || (numConfigs != 1))
	{
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
	surface = eglCreateWindowSurface(
		display, config, global_data->native_window, NULL
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
(struct myy_window_parameters const * __restrict const window_params,
 struct _escontext * __restrict const global_data)
{
	int ret = CreateNativeWindow(window_params, global_data);
	if (ret) {
		return EGL_FALSE;
	}
	return CreateEGLContext(window_params, global_data);
}


/* If too many values like this exist, a structure will be needed */
unsigned long last_click = 0;
struct is_moving {
	uint32_t button;
	uint16_t start_x, start_y;
} is_moving = {0};


void ParseEvents
(xcb_connection_t * const connection)
{

	xcb_generic_event_t * event;

	while ((event = xcb_poll_for_event(connection))) {
		unsigned int response = (event->response_type & ~0x80);
		switch(response) {
			case XCB_CLIENT_MESSAGE: {
					// Terrible hack. We should check the message content.
					// That said, we only listen for close events so...
				myy_user_quit();
			}
			break;
			case XCB_RESIZE_REQUEST: {
				xcb_resize_request_event_t const * resize =
					(xcb_resize_request_event_t const *) event;
				if (resize->width > 0 && resize->height > 0)
					myy_display_initialised(resize->width, resize->height);
					
			}
			break;
			case XCB_BUTTON_PRESS: {
				// Values definitions after a label borks the compiler
				xcb_button_press_event_t *bp =
					(xcb_button_press_event_t *) event;
				unsigned int
					x = bp->event_x,
					y = bp->event_y,
					button = bp->detail;
				unsigned long click_time = bp->time;

				if (is_moving.button == 0) {
					is_moving.button = button;
					is_moving.start_x = x;
					is_moving.start_y = y;
				}
				if (click_time - last_click > 250)
					myy_click(x, y, button);
				else myy_doubleclick(x, y, button);
					last_click = click_time;
			}
			break;
			case XCB_BUTTON_RELEASE: {
				is_moving.button = 0;
			}
			break;
			case XCB_MOTION_NOTIFY: {
				xcb_motion_notify_event_t * motion =
					(xcb_motion_notify_event_t *) event;

				if (is_moving.button == 0)
					myy_hover(motion->event_x, motion->event_y);
				else
					myy_move(
						motion->event_x, motion->event_y,
						is_moving.start_x, is_moving.start_y
					);
			}
			break;
			case XCB_KEY_PRESS: {
				// Keyboards values are shifted by 3 with X11, for
				// 'historical' reasons.
				xcb_key_press_event_t * kp =
					(xcb_key_press_event_t *) event;
				myy_key(kp->detail - 8);
			}
			break;
			case XCB_KEY_RELEASE: {
				xcb_key_press_event_t * kp =
					(xcb_key_press_event_t *) event;
				myy_key_release(kp->detail - 8);
			}
			break;
			case XCB_DESTROY_NOTIFY:
			case XCB_UNMAP_NOTIFY: {
				LOG("Blargh ! Deading !\n");
			}
			break;
		}
		free(event);
	}

	return;
}

void Terminate
(Display * display, Window window)
{
	if (window) XDestroyWindow( display, window );
	XCloseDisplay( display );
}

