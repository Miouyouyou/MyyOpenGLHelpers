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
#include "init_window.h"

#include <string.h>
#include <myy/current/opengl.h>
#include <myy/helpers/log.h>
#include <myy/helpers/memory.h>

#include <locale.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>

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

#define TRUE 1
#define FALSE 0

int CreateNativeWindow(
	struct myy_window_parameters const * __restrict const window_params,
	struct _escontext * __restrict const global_data,
	void * implementation_details)
{
	Window root;
	XSetWindowAttributes swa;
	XSetWindowAttributes xattr;
	Atom wm_state;
	XWMHints hints;
	XEvent xev;
	Window win;
	setlocale(LC_ALL, "");
	XSetLocaleModifiers("");
	Display* x_display = XOpenDisplay(NULL);
	int const width  = window_params->width;
	int const height = window_params->height;
	char const * __restrict const title = window_params->title;
	Bool detectable_autorepeat_supported = FALSE;
	struct myy_xlib_state * __restrict const xlib_state =
		implementation_details;

	root = DefaultRootWindow(x_display);

	/* XCreateWindow arguments :
		Display,
		Parent window,
		x, y from Parent Window
		width, height of the new window,
		Border width,
		Depth,
		Window class,
		VisualType,
		MasksType,
		Masks */

	win = XCreateSimpleWindow(
		x_display,
		root,
		0, 0,
		width, height,
		0,
		0,
		0);

	long const mask = 
		ExposureMask
		| ButtonPressMask
		| ButtonReleaseMask
		| ButtonMotionMask
		| PointerMotionMask
		| KeyPressMask
		| KeyReleaseMask
		| FocusChangeMask;
	XSelectInput(x_display, win, mask);
	
	LOG("Open the IM\n");
	
	XIM xim = XOpenIM(x_display, NULL, NULL, NULL);
	if (xim == NULL) {

		LOG("Failed...\n");
		/* Secret ancestral technique picked from there :
		 * https://qiita.com/ai56go/items/63abe54f2504ecc940cd
		 * I don't know if this fallback works everytime
		 * though...
		 */
		LOG("Retrying with @im=none\n");
		XSetLocaleModifiers("@im=none");
		xim = XOpenIM(x_display, 0, 0, 0);
	}

	XIC xic = NULL;
	if (xim != NULL) {
		LOG("Trying XCreateIC\n");
		xic = XCreateIC(xim,
			/* The definition of XCreateIC is
			 * XCreateIC(XIM, ...)
			 * The rest is just a set of Key/Values,
			 * ended with NULL.
			 * EGL uses a similar way, using an
			 * array, instead of Variadic Arguments.
			 */
			XNClientWindow, win,
			XNFocusWindow, win,
			XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
			NULL);
		if (xic == NULL) {
			LOG("XCreateIC returned NULL. No IME for you !\n");
			XCloseIM(xim);
			xim = NULL;
		}
		else {
			LOG("XCreateIC Success !\n");
			XSetICFocus(xic);
		}
	}
	LOG("Went there, done that\n");
	xlib_state->xic = xic;
	xlib_state->xim = xim;
	xlib_state->compose_buffer =
		allocate_durable_memory(4096);

	if (xlib_state->compose_buffer == NULL) {
		if (xic != NULL) XDestroyIC(xic);
		if (xim != NULL) XCloseIM(xim);
		return 1;
	}

	LOG("Window : %ld\n", win);

	memset(xlib_state->compose_buffer, 0, 4096);
	/* Without this, the window would not be visible
		Quoting XChangeWindowAttributes manual :
	The created window is not yet displayed (mapped) on the user's
	display. To display the window, call XMapWindow. */

	XMapWindow(x_display, win);

	/* Sets the title. Can it be done before Mapping the window ? */
	XStoreName(x_display, win, title);

	/* ???? */
	/* Get the ATOM "_NET_WM_STATE",
	 * Create it if it doesn't exist
     *
	 *   The XInternAtom function returns the atom identifier associated
	 *   with the specified atom_name string.  If only_if_exists is False,
	 *   the atom is created if it does not exist.
	 */
	wm_state = XInternAtom(x_display, "_NET_WM_STATE", FALSE);

	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage; /* XClientMessageEvent */
	xev.xclient.window       = win;
	xev.xclient.message_type = wm_state;
	xev.xclient.format       = 32;
	/* _NET_WM_STATE_REMOVE        0    remove/unset property
	 * _NET_WM_STATE_ADD           1    add/set property
	 * _NET_WM_STATE_TOGGLE        2    toggle property
	 */
	xev.xclient.data.l[0]    = 1;
	xev.xclient.data.l[1]    = FALSE;

	XSendEvent (
		x_display,
		root, // The example states DefaultRootWindow( x_display ) ??
		FALSE,
		SubstructureNotifyMask,
		&xev );

	/* So, this seems to be needed in order to catch the destruction of
	 * the window, when scanning for events
	 * Don't do this and you get a nice 'fatal IO error 11 BUG' when
	 * closing the window.
	 */

	destroy = XInternAtom(x_display, "WM_DELETE_WINDOW", FALSE);
	XSetWMProtocols(x_display, win, &destroy, 1);

	/* Let's make AutoRepeat "detectable".
	 * 
	 * Without that, if you keep a key pushed, the window will receive :
	 * KeyPress, KeyRelease, KeyPress, KeyRelease.
	 * With that function, the window will receive :
	 * KeyPress, KeyPress, ..., KeyRelease
	 * Which makes autorepeat "Detectable".
	 */
	XkbSetDetectableAutoRepeat(
		x_display, 1,
		&detectable_autorepeat_supported);
	LOG("Detectable autorepeat supported ? %d\n",
		detectable_autorepeat_supported);

	global_data->native_display = x_display;
	global_data->window_width   = width;
	global_data->window_height  = height;
	global_data->native_window  = (EGLNativeWindowType) win;

	XSync(x_display, False);

	return 0;
}

EGLBoolean CreateEGLContext(
	struct myy_window_parameters const * __restrict const parameters,
	struct _escontext * __restrict const global_data,
	void * implementation_details)
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



EGLBoolean CreateWindowWithEGLContext(
	struct myy_window_parameters const * __restrict const window_params,
	struct _escontext * __restrict const global_data,
	void * implementation_details)
{
	int ret = CreateNativeWindow(
		window_params, global_data,
		implementation_details);
	if (ret)
		return EGL_FALSE;

	return CreateEGLContext(
		window_params, global_data,
		implementation_details);
}

void ParseEvents(
	Display * x_display,
	struct myy_input_state * __restrict const input_state,
	void * implementation_details)
{

	XEvent xev;
	struct myy_xlib_state const * __restrict const xlib_state =
		implementation_details;

	while ( XPending( x_display ) ) {
		XNextEvent( x_display, &xev );
		if (XFilterEvent(&xev, None) == True) continue;
		switch(xev.type) {
			case ClientMessage:
				if (xev.xclient.data.l[0] == destroy)
					myy_user_quit();
				break;
			case ResizeRequest:
				myy_display_initialised(
					xev.xresizerequest.width,
					xev.xresizerequest.height);
				break;
			case FocusIn:
				LOG("FocusIN\n");
				Window win;
				XSetICFocus(xlib_state->xic);
				XGetICValues(xlib_state->xic, XNFocusWindow, &win, NULL);
				LOG("Window : %ld\n", win);
				XGetICValues(xlib_state->xic, XNClientWindow, &win, NULL);
				LOG("Window : %ld\n", win); 
				break;
			case FocusOut:
				LOG("FocusOUT\n");
				XUnsetICFocus(xlib_state->xic);
				break;
			case ButtonPress:
				;
				/* Definitions after a label borks C.
				 * Note that it also breaks with goto labels so...
				 * It's a C thing.
				 */
				unsigned int const x      = xev.xbutton.x;
				unsigned int const y      = xev.xbutton.y;
				unsigned int const button = xev.xbutton.button;
				unsigned long click_time  = xev.xbutton.time;

				if (!input_state->move_click.start_button)
				{
					input_state->move_click.start_button = button;
					input_state->move_click.start_x = x;
					input_state->move_click.start_y = y;
				}
				/* TODO Let the application decide by passing the input state. */
				if (click_time - input_state->last_click_ms > 250)
					myy_click(x, y, button);
				else
					myy_doubleclick(x, y, button);
				input_state->last_click_ms = click_time;

				break;
			case ButtonRelease:
				input_state->move_click.start_button = 0;
				break;
			case MotionNotify:
				/* TODO Let the application decide by passing the input state */
				if (!input_state->move_click.start_button)
					myy_hover(xev.xmotion.x, xev.xmotion.y);
				else
					myy_move(
						xev.xmotion.x, xev.xmotion.y,
						input_state->move_click.start_x,
						input_state->move_click.start_y);
				break;
			case KeyPress:
				//myy_key(xev.xkey.keycode);
				//XSetICFocus(xlib_state->xic);
				;
				KeySym keysym = NoSymbol;
				Status lookup_status;
				int len = Xutf8LookupString(
					xlib_state->xic, &xev.xkey,
					xlib_state->compose_buffer,
					4095,
					&keysym, &lookup_status);
				LOG("lookup_status : %d\n", lookup_status);

				if (len > 0)
					myy_text(xlib_state->compose_buffer, len);

				break;
			case KeyRelease:
				myy_key_release(xev.xkey.keycode);
				break;
			case DestroyNotify:
			case UnmapNotify:
				LOG("Blargh !");
				break;
		}
	}

}

void Terminate(
	Display * display,
	Window window,
	void * implementation_details)
{
	struct myy_xlib_state * __restrict const xlib_state =
		implementation_details;

	if (window) XDestroyWindow( display, window );

	if (xlib_state->xic) XDestroyIC(xlib_state->xic);
	if (xlib_state->xim) XCloseIM(xlib_state->xim);
	free_durable_memory(xlib_state->compose_buffer);

	XCloseDisplay( display );
}
