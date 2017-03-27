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

struct _escontext ESContext = {
  .native_display = NULL,
  .window_width = 0,
  .window_height = 0,
  .native_window  = 0,
  .display = NULL,
  .context = NULL,
  .surface = NULL
};

#define TRUE 1
#define FALSE 0

void CreateNativeWindow
(const char * __restrict const title, const int width, const int height)
{
  Window root;
  XSetWindowAttributes swa;
  XSetWindowAttributes xattr;
  Atom wm_state;
  XWMHints hints;
  XEvent xev;
  Window win;
  Atom wm_delete;
  Display* x_display = XOpenDisplay(NULL);

  root = DefaultRootWindow(x_display);

  swa.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | KeyPressMask;

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

  win = XCreateWindow(x_display, root, 0, 0, width, height,
                      0, CopyFromParent, InputOutput, CopyFromParent,
                      CWEventMask, &swa);

  xattr.override_redirect = FALSE;

  XChangeWindowAttributes( x_display, win, CWOverrideRedirect, &xattr );

  /* Tells the window manager to accept input for this new window */
  hints.input = TRUE;
  hints.flags = InputHint;

  XSetWMHints(x_display, win, &hints);

  /* Without this, the window would not be visible
     Quoting XChangeWindowAttributes manual :
   The created window is not yet displayed (mapped) on the user's
   display. To display the window, call XMapWindow. */

  XMapWindow(x_display, win);

  /* Sets the title. Can it be done before Mapping the window ? */
  XStoreName(x_display, win, title);

  /* Get the ATOM "_NET_WM_STATE",
     Create it if it doesn't exist

   The XInternAtom function returns the atom identifier associated with
   the specified atom_name string.  If only_if_exists is False, the atom
   is created if it does not exist. */
  wm_state = XInternAtom(x_display, "_NET_WM_STATE", FALSE);

  memset(&xev, 0, sizeof(xev));
  xev.type = ClientMessage; /* XClientMessageEvent */
  xev.xclient.window       = win;
  xev.xclient.message_type = wm_state;
  xev.xclient.format       = 32;
  /* _NET_WM_STATE_REMOVE        0    remove/unset property
     _NET_WM_STATE_ADD           1    add/set property
     _NET_WM_STATE_TOGGLE        2    toggle property  */
  xev.xclient.data.l[0]    = 1;
  xev.xclient.data.l[1]    = FALSE;
  XSendEvent (
    x_display,
    root, // The example states DefaultRootWindow( x_display ) ??
    FALSE,
    SubstructureNotifyMask,
    &xev );

  /* So, this seems to be needed in order to catch the destruction of
     the window, when scanning for events
     The lines topped with a 'fatal IO error 11 BUG' concern this
     problem */

  destroy = XInternAtom(x_display, "WM_DELETE_WINDOW", FALSE);
  XSetWMProtocols(x_display, win, &destroy, 1);

  ESContext.native_display = x_display;
  ESContext.window_width = width;
  ESContext.window_height = height;
  ESContext.native_window = (EGLNativeWindowType) win;
}

EGLBoolean CreateEGLContext ()
{
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

   EGLDisplay display = eglGetDisplay( ESContext.native_display );
   if ( display == EGL_NO_DISPLAY ) return EGL_FALSE;

   // Initialize EGL
   if ( !eglInitialize(display, &majorVersion, &minorVersion) )
     return EGL_FALSE;

   // Get configs
   if (   (eglGetConfigs(display, NULL, 0, &numConfigs) != EGL_TRUE)
       || (numConfigs == 0))
     return EGL_FALSE;

   // Choose config
   if ( (eglChooseConfig(display, eglAttribs,
                         &config, 1, &numConfigs) != EGL_TRUE)
       || (numConfigs != 1))
     return EGL_FALSE;

   // Create a surface
   surface =
     eglCreateWindowSurface(display, config,
                            ESContext.native_window, NULL);

   if ( surface == EGL_NO_SURFACE ) return EGL_FALSE;

   // Create a GL context
   context = eglCreateContext(display, config,
                              EGL_NO_CONTEXT, contextAttribs );

   if ( context == EGL_NO_CONTEXT ) return EGL_FALSE;

   // Make the context current
   if ( !eglMakeCurrent(display, surface, surface, context) )
     return EGL_FALSE;

   ESContext.display = display;
   ESContext.surface = surface;
   ESContext.context = context;
   return EGL_TRUE;
}

void RefreshWindow() {
  eglSwapBuffers(ESContext.display, ESContext.surface);
}

EGLBoolean CreateWindowWithEGLContext
(const char * __restrict const title,
 const int width, const int height) {
  CreateNativeWindow(title, width, height);
  return CreateEGLContext();
}


/* If too many values like this exist, a structure will be needed */
unsigned long last_click = 0;
struct is_moving {
	uint32_t button;
	uint16_t start_x, start_y;
} is_moving = {0};

unsigned int UserInterrupt() {

  XEvent xev;
  Display *x_display = ESContext.native_display;
  unsigned int interrupted = 0;

  while ( XPending( x_display ) ) {
    XNextEvent( x_display, &xev );
    switch(xev.type) {
      case ClientMessage:
        interrupted = (xev.xclient.data.l[0] == destroy);
        break;
      case ButtonPress:
        ; // Values definitions after a label borks the compiler
        unsigned int
          x = xev.xbutton.x,
          y = ESContext.window_height - xev.xbutton.y,
          button = xev.xbutton.button;
        unsigned long click_time = xev.xbutton.time;

				if (is_moving.button == 0) {
					is_moving.button = button;
				  is_moving.start_x = x;
					is_moving.start_y = y;
				}
        if (click_time - last_click > 250) myy_click(x, y, button);
        else myy_doubleclick(x, y, button);
        last_click = click_time;
        break;
			case ButtonRelease: {
				is_moving.button = 0;
				break;
			}
      case MotionNotify:
        if (is_moving.button == 0)
					myy_hover(
						xev.xmotion.x, ESContext.window_height - xev.xmotion.y
					);
				else
					myy_move(
						xev.xmotion.x, ESContext.window_height - xev.xmotion.y,
						is_moving.start_x, is_moving.start_y
					);
        break;
      case KeyPress:
        myy_key(xev.xkey.keycode);
        break;
    }
  }

  return interrupted;
}

static void Terminate() {
  Display *display = ESContext.native_display;
  Window window = ESContext.native_window;

  if (window) XDestroyWindow( ESContext.native_display, window );

  XCloseDisplay( display );
}
