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
	Atom wm_state;
	XEvent xev;
	Window win;

	/* You'd think that these two calls, done with an empty string,
	 * would just generate a "reset to the default status" or
	 * overwrite some status with nothing.
	 * 
	 * Turns out that it's not the case.
	 * If you launch setLocale(LC_ALL, "") it just setup LC_ALL
	 * locale category to the one defined in the environment variable.
	 * Also, for XSetLocaleModifiers(3) :
	 * 
	 *   The local host X locale modifiers announcer
	 *   (on POSIX-compliant systems, the XMODIFIERS environment variable)
	 *   is appended to the modifier_list to provide default values on the
	 *   local host. 
	 * 
	 * ...
	 * 
	 * OBVIOUSLY ! Yeah !
	 * 
	 * So basically, these two calls use environment variables to
	 * setup the environment...
	 * Because, yes, environment variables are not used automatically.
	 * 
	 * It's things like this that make you realize that a lot of things
	 * you thought automatic... are not...
	 * 
	 * So basically, if I don't call setlocale(LC_ALL, ""), then if you
	 * launch the software with, say, LC_ALL = ja_JP.utf8 , it will be
	 * ignored and, even if the IME triggers, you'll probably not be
	 * able to convert the input characters because the converting
	 * functions won't be set on the XIM layer...
	 * 
	 * And if I don't call XSetLocaleModifiers("") , then if you
	 * launch the software with, say, XMODIFIERS=@im=fcitx then it will
	 * be ignored too, and you won't get any IME support from FCITX,
	 * because no connection will be done from the XIM client, which is
	 * our application and the XIM server, which is FCITX in this case.
	 * So instead, you'll just see FCITX says that no input window is
	 * currently focused when focusing this application.
	 * 
	 * Seriously...
	 */
	setlocale(LC_ALL, "");
	XSetLocaleModifiers("");
	
	Display* x_display = XOpenDisplay(NULL);
	int const width  = window_params->width;
	int const height = window_params->height;
	char const * __restrict const title = window_params->title;
	Bool detectable_autorepeat_supported = FALSE;
	struct myy_xlib_state * __restrict const xlib_state =
		(typeof(xlib_state)) implementation_details;

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
	xlib_state->xic = xic;
	xlib_state->xim = xim;
	xlib_state->compose_buffer =
		(typeof(xlib_state->compose_buffer))
		allocate_durable_memory(4096);

	if (xlib_state->compose_buffer == NULL) {
		if (xic != NULL) XDestroyIC(xic);
		if (xim != NULL) XCloseIM(xim);
		return 1;
	}

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
	myy_states * __restrict const states,
	Display * x_display,
	void * implementation_details)
{

	XEvent xev;
	struct myy_xlib_state const * __restrict const xlib_state =
		(typeof(xlib_state)) implementation_details;

	while ( XPending( x_display ) ) {
		XNextEvent( x_display, &xev );
		if (XFilterEvent(&xev, None) == True) continue;

		enum myy_input_events event_type = myy_input_event_invalid;
		union myy_input_event_data data;
		switch(xev.type) {
			case ClientMessage:
			{
				if (xev.xclient.data.l[0] == destroy) {
					event_type = myy_input_event_window_destroyed;
				}
			}
			break;
			case ResizeRequest:
			{
				event_type = myy_input_event_surface_size_changed;
				data.surface.width  = xev.xresizerequest.width;
				data.surface.height = xev.xresizerequest.height;
			}
			break;
			case FocusIn:
			{
				event_type = myy_input_event_window_focus_in;
				LOG("FocusIN\n");
			}
			break;
			case FocusOut:
			{
				event_type = myy_input_event_window_focus_out;
				LOG("FocusOUT\n");
			}
			break;
			case ButtonPress:
			case ButtonRelease:
			{
				event_type = 
					(xev.type == ButtonPress)
					? myy_input_event_mouse_button_pressed
					: myy_input_event_mouse_button_released;

				data.mouse_button.x     = xev.xbutton.x;
				data.mouse_button.y     = xev.xbutton.y;
				data.mouse_button.index = 0;
				data.mouse_button.state = 0;
				data.mouse_button.button_number = xev.xbutton.button;
			}
			break;
			case MotionNotify:
			{
				event_type = myy_input_event_mouse_moved_absolute;
				data.mouse_move_absolute.x = xev.xmotion.x;
				data.mouse_move_absolute.y = xev.xmotion.y;
				data.mouse_move_absolute.index = 0;
				data.mouse_move_absolute.type  = 1;
			}
			break;
			case KeyPress:
			{
				//myy_key(xev.xkey.keycode);
				//XSetICFocus(xlib_state->xic);
				;
				KeySym keysym = NoSymbol;
				Status lookup_status;
				int len = Xutf8LookupString(
					xlib_state->xic, &xev.xkey,
					xlib_state->compose_buffer,
					4094,
					&keysym, &lookup_status);
				xlib_state->compose_buffer[len] = 0;
				LOG("lookup_status : %d\n", lookup_status);

				switch(lookup_status) {
					case XBufferOverflow: break;
					case XLookupNone: break;
					case XLookupChars:
						event_type = myy_input_event_text_received;
						data.text.data   = xlib_state->compose_buffer;
						data.text.length = len;
						break;
					case XLookupKeySym:
						event_type         = myy_input_event_keyboard_key_pressed;
						data.key.raw_code  = xev.xkey.keycode-8;
						data.key.modifiers = 0;
						data.key.state     = 0;
						myy_input(states, event_type, &data);

						event_type         = myy_input_event_keyboard_key_released;
						data.key.state     = 1;
						break;
					case XLookupBoth:
						event_type = myy_input_event_text_received;
						data.text.data   = xlib_state->compose_buffer;
						data.text.length = len;
						myy_input(states, event_type, &data);

						event_type         = myy_input_event_keyboard_key_pressed;
						data.key.raw_code  = xev.xkey.keycode-8;
						data.key.modifiers = 0;
						data.key.state     = 0;
						myy_input(states, event_type, &data);
						break;
				}
			}
			break;
			case KeyRelease:
			{
				event_type         = myy_input_event_keyboard_key_released;
				data.key.raw_code  = xev.xkey.keycode-8;
				data.key.modifiers = 0;
				data.key.state     = 1;
			}
			break;
			case DestroyNotify:
			case UnmapNotify:
			{
				event_type = myy_input_event_window_destroyed;
				LOG("Blargh !");
			}
			break;
		}
		myy_input(states, event_type, &data);
	}

}

void myy_text_input_start(
	myy_states * __restrict const state)
{
	struct myy_xlib_state * __restrict const xlib_state =
		(typeof(xlib_state)) (state->platform_state);
	XSetICFocus(xlib_state->xic);
}

void myy_text_input_stop(
	myy_states * __restrict const state)
{
	struct myy_xlib_state * __restrict const xlib_state =
		(typeof(xlib_state)) (state->platform_state);
	XUnsetICFocus(xlib_state->xic);
}

void Terminate(
	Display * display,
	Window window,
	void * implementation_details)
{
	struct myy_xlib_state * __restrict const xlib_state =
		(typeof(xlib_state)) implementation_details;

	if (window) XDestroyWindow( display, window );

	if (xlib_state->xic) XDestroyIC(xlib_state->xic);
	if (xlib_state->xim) XCloseIM(xlib_state->xim);
	free_durable_memory(xlib_state->compose_buffer);

	XCloseDisplay( display );
}
