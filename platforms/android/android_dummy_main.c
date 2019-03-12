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

#include <jni.h>
#include <errno.h>

#include <android/log.h>
#include "android_native_app_glue.h"

#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h> /* chdir */

/* Bad mkdir */
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>

#include <myy/myy.h>
#include <myy/current/opengl.h>
#include <myy/helpers/macros.h>
#include <myy/helpers/vector.h>

#ifdef DEBUG
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))
#else
#define LOGI(...)
#define LOGW(...)
#endif // DEBUG

static struct egl_elements {
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
} egl;

static struct current_window {
	uint16_t width, height;
} current_android_window;

AAssetManager *myy_assets_manager;

/* TODO REMOVE THIS SHIT ! */
struct android_app * ugly_pointer = NULL;

myy_vector_template(utf8, uint8_t)

myy_vector_utf8 received_string;

void myy_editor_finished(uint8_t const * __restrict const array, size_t array_length);
static void android_app_write_cmd(struct android_app* android_app, int8_t cmd) {
    if (write(android_app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd)) {
        LOGW("Failure writing android_app cmd: %s\n", strerror(errno));
    }
}
/* Turns out that the JNI isn't able to produce valid UTF-8 strings.
 * So we have to resort to that kind of hacks where we convert
 * the string UTF-8 bytes arrays inside the Java application, and
 * pass the byte array to the native code...
 * 
 * Everything is great in Java.
 */
void Java_com_miouyouyou_gametests_NativeInsanity_myyTextInputStopped(
	JNIEnv * __restrict const jni,
	jobject object,
	jbyteArray provided_string)
{
	myy_vector_utf8_reset(&received_string);
	jbyte * __restrict const utf8_string =
		(*jni)->GetByteArrayElements(jni, provided_string, NULL);
	size_t const utf8_string_size =
		(*jni)->GetArrayLength(jni, provided_string);

	myy_vector_utf8_add(&received_string,
		utf8_string_size,
		(uint8_t const * __restrict) utf8_string);
	uint8_t append = 0;
	myy_vector_utf8_add(&received_string, 1, &append);
	android_app_write_cmd(ugly_pointer, MYY_APP_CMD_EDITOR_SENT_TEXT);
}

/**
 * Initialize an EGL context for the current display.
 */
static int add_egl_context_to(
	NativeWindowType const window,
	struct egl_elements * __restrict const e,
	struct current_window * __restrict const window_infos)
{
	// initialize OpenGL ES and EGL

	/*
	 * Here specify the attributes of the desired configuration.
	 * Below, we select an EGLConfig with at least 8 bits per color
	 * component compatible with on-screen windows
	 */
	EGLint w, h, dummy, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;
	EGLint eglAttribs[] = {
		MYY_EGL_COMMON_MOBILE_ATTRIBS,
		EGL_NONE, EGL_NONE
	};
	EGLint contextAttribs[] =
		{ MYY_CURRENT_GL_CONTEXT, EGL_NONE };

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* Here, the application chooses the configuration it desires.
	 * In this sample, we have a very simplified selection process,
	 * where we pick the first EGLConfig that matches our criteria
	 */
	eglChooseConfig(display, eglAttribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	 * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	 * As soon as we picked a EGLConfig, we can safely reconfigure the
	 * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID.
	 */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	/* Implicitly provided by EGL on Android */
	ANativeWindow_setBuffersGeometry(window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, window, NULL);
	context = eglCreateContext(display, config, NULL, contextAttribs);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
		return -1;

	e->context = context;
	e->display = display;
	e->surface = surface;

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	window_infos->width  = (uint16_t) w;
	window_infos->height = (uint16_t) h;

	return 0;
}

static void egl_sync(struct egl_elements* const e) {
	eglSwapBuffers(e->display, e->surface);
}

static void egl_stop(struct egl_elements* const e) {
	if (e->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(e->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (e->context != EGL_NO_CONTEXT)
			eglDestroyContext(e->display, e->context);
		if (e->surface != EGL_NO_SURFACE)
			eglDestroySurface(e->display, e->surface);
		eglTerminate(e->display);
	}
	e->display = EGL_NO_DISPLAY;
	e->context = EGL_NO_CONTEXT;
	e->surface = EGL_NO_SURFACE;
}

void myy_cleanup_drawing() {
	egl_stop(&egl);
}


/**
 * Shared state for our app.
 */

int animating;

/**
 * Process the next input event.
 */
unsigned long last_tap = 0;
int16_t start_x = 0, start_y = 0;
uint64_t flags = 0;
static int32_t engine_handle_input(
	struct android_app * const app,
	AInputEvent * const event)
{
	if (flags) return 0;
	unsigned int const action = AMotionEvent_getAction(event);

	if (action != AMOTION_EVENT_ACTION_DOWN &
		action != AMOTION_EVENT_ACTION_MOVE)
	{
		LOG("******** ?? Action : %d ?? ********\n", action);
		return 0;
	}
	else {
		int32_t flags = AMotionEvent_getFlags(event);
		LOG("******** !! Action : %d (flags : %d) !! ********\n", action, flags);
		
	}
	//unsigned long const tap_time = AMotionEvent_getEventTime(event);
	int const x = AMotionEvent_getX(event, 0);
	int const y = AMotionEvent_getY(event, 0);


	switch(action) {
	case AMOTION_EVENT_ACTION_DOWN:
		start_x = x;
		start_y = y;
		//if (tap_time - last_tap > 0x10000000)
			myy_click(x, y, 1);
		//else 
		//	myy_doubleclick(x, y, 1);
		//last_tap = tap_time;
		break;
	case AMOTION_EVENT_ACTION_MOVE:
		myy_move(x, y, start_x, start_y);
		break;
	}

	return 1;
}

static void goto_data_dir(const char* data_dir) { chdir(data_dir); }

struct myy_game_state game_state = {0};

/* A hidden global state pointer, how nice ! */
ANativeActivity * myy_android_activity;
/**
 * Process the next main command.
 */
static void engine_handle_cmd(
	struct android_app * const app, 
	int32_t const cmd)
{

	struct egl_elements *e = &egl;
	struct myy_game_state *state = &game_state;
	switch (cmd) {
	case APP_CMD_INIT_WINDOW:
		LOGW("======================================");
		LOGW("========= Initialising window");
		if (app->window != NULL)
			add_egl_context_to(app->window, e, &current_android_window);
		myy_init_drawing();
		myy_display_initialised(
			current_android_window.width,
			current_android_window.height);
		animating = 1;
		break;
	case APP_CMD_WINDOW_RESIZED:
		LOGW("========= Resizing !");
		animating = 0;
		egl_stop(e);
		add_egl_context_to(app->window, e, &current_android_window);
		animating = 1;
	case APP_CMD_RESUME:
		LOGW("========= Resuming !");
		myy_android_activity = app->activity;
		myy_resume_state(state);
		break;
	case APP_CMD_PAUSE:
		LOGW("========= Pause !");
		animating = 0;
		myy_save_state(state);
		break;
	case APP_CMD_STOP:
		myy_stop();
		break;
	case APP_CMD_SAVE_STATE:
		LOGW("========= State saved !");
		myy_save_state(state);
		break;
	case APP_CMD_TERM_WINDOW:
		LOGW("========= Terminated !");
		myy_cleanup_drawing();
		myy_stop();
		egl_stop(e);
	case MYY_APP_CMD_EDITOR_SENT_TEXT:
		myy_editor_finished(
			myy_vector_utf8_data(&received_string),
			myy_vector_utf8_length(&received_string));
		flags = 0;
		break;
	}

}

/* Got to love the JNI taxonomy :
 *
 *  _JNIEnv is a structure which first member, 'functions' is defined
 * as struct const JNINativeInterface * .
 * However ! JNIEnv is a typedef to struct const JNINativeInterface * !
 * SO, the first member of _JNIEnv could also be defined as :
 * JNIEnv functions
 *
 *  So JNIEnv and _JNIEnv are not the same !
 *  JNIEnv != _JNIEnv
 *  typeof(_JNIEnv->functions) == typeof(JNIEnv)
 *
 *  Adding to this, The JVM wants JNIEnv pointer as argument for most
 * of its helpers methods.
 * So it basically wants a : struct JNINativeInterface ** identifier
 *
 * Why is it so convoluted ? Because Java.
 */
struct android_calling_kit {
	JNIEnv jni_helpers;
	JNIEnv * env;
	jobject java_activity;
	jclass activity_class;
	JavaVM * java_vm;
};

struct android_calling_kit prepare_java_call(
	const ANativeActivity * __restrict const android_activity)
{

	// "This member (clazz) is mis-named. It should really be named
	//  'activity' instead of 'clazz', since it's a reference to the
	//  NativeActivity instance created by the system for you."
	//  (© native_activity.h)
	jobject activity_java_object = android_activity->clazz;

	JavaVM * vm = android_activity->vm;
	JNIEnv * jenv;
	(*vm)->AttachCurrentThread(vm, &jenv, NULL);

	JNIEnv helpers_functions = *jenv;

	jclass activity_java_class =
		helpers_functions->GetObjectClass(jenv, activity_java_object);

	struct android_calling_kit j = {
		.jni_helpers = helpers_functions,
		.env = jenv,
		.java_activity = activity_java_object,
		.activity_class = activity_java_class,
		.java_vm = vm
	};

	return j;
}
void myy_open_website(const char * __restrict const url) {

	LOGW("[myy_open_website] !!");
	struct android_calling_kit android =
		prepare_java_call(myy_android_activity);

	/* We won't build the Java String from C, but instead pass the
	* bytes directly in an array, and let the JVM produces a String
	* from these bytes directly
	*/
	/* Java Byte[] */
	unsigned int const url_size = strlen(url);
	// jbyteArray const ... -> void * const ...
	jbyteArray const url_as_java_byte_array =
		android.jni_helpers->NewByteArray(android.env, url_size);
	android.jni_helpers->SetByteArrayRegion(
		android.env, url_as_java_byte_array, 0, url_size,
		(const jbyte *) url);

	const char * const java_method_name = "openWebsite";
	const char * const java_method_signature = "([B)V";

	jmethodID activity_OpenWebsite_meth =
		android.jni_helpers->GetMethodID(
			android.env, android.activity_class,
			java_method_name, java_method_signature);

	if (activity_OpenWebsite_meth != NULL) {
		LOGW("Opening website..., methodID address : %p\n",
			activity_OpenWebsite_meth);
		android.jni_helpers->CallVoidMethod(
			android.env, android.java_activity,
			activity_OpenWebsite_meth, url_as_java_byte_array);
	}
	else 
		LOGW("You're sure about that method name : %s %s\n",
			java_method_name, java_method_signature);

	// TODO Document this
	(*android.java_vm)->DetachCurrentThread(android.java_vm);

}

void myy_stop()
{
	animating = 0;
	/* TODO Can't this crash the whole thing ?
	 * Removing the whole display while operations are
	 * still going on might not be the best way to
	 * handle this.
	 * We could throw a kill signal to the thread after
	 * being done though...
	 * 
	 * We clearly need to do some accounting to know what
	 * ressources are used by the android "glue" and which
	 * one could cause any issue, if we end up the thread
	 * abruptly.
	 */
	myy_cleanup_drawing();
	myy_save_state(&game_state);
}

void myy_user_quit()
{
	/* TODO
	 * Check if it's not better to use the Android event system
	 * instead. This out-of-band way might not be the best for
	 * such systems.
	 */
	myy_stop();
}




/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* app) {

	/* TODO How to generate serious issues in one lesson */
	ugly_pointer = app;
	received_string = myy_vector_utf8_init(4096);

	// Make sure glue isn't stripped.
	app_dummy();

	LOG("!!!!!!!!!!!!!!??????????????????????:!!!!!!!!!!!!!!!§????????????????? gettid() : %x\n", gettid());
	app->onAppCmd = engine_handle_cmd;
	app->onInputEvent = engine_handle_input;

	myy_android_activity = app->activity;
	myy_assets_manager = app->activity->assetManager;

	// loop waiting for stuff to do.
	LOGW("myy_open_website address : %p\n", myy_open_website);

	struct egl_elements* e = &egl;
	while (1) {
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		ident = ALooper_pollAll(
			animating ? 0 : -1,
			NULL,
			&events,
			(void**) &source);

		/* Parse events */
		while (ident >= 0) {

			if (app->destroyRequested != 0) {
				myy_stop();
				return;
			}

			// Process this event.
			if (source != NULL) { source->process(app, source); }

			ident = ALooper_pollAll(
				animating ? 0 : -1,
				NULL,
				&events,
				(void**) &source);
		}

		/* Draw */
		if (animating && app->window != NULL) {
			// Drawing is throttled to the screen update rate, so there
			// is no need to do timing here.
			myy_draw();
			egl_sync(e);
		}
	}
}

void myy_trigger_text_input() {
	LOG("Mon hamster fait du kung-fu sauvage transalpin !\n");

	LOGW("[myy_trigger_text_input] !!");
	struct android_calling_kit android =
		prepare_java_call(myy_android_activity);

	const char * const java_method_name = "startInput";
	const char * const java_method_signature = "(JI)V";

	jmethodID activity_startInput_meth =
		android.jni_helpers->GetMethodID(
			android.env, android.activity_class,
			java_method_name, java_method_signature);

	if (activity_startInput_meth != NULL) {
		LOGW("Starting input..., methodID address : %p\n",
			activity_startInput_meth);
		LOGW("Passing game_state : %p\n", &game_state);
		flags = 1;
		android.jni_helpers->CallVoidMethod(
			android.env, android.java_activity,
			activity_startInput_meth, &game_state, 0);
	}
	else 
		LOGW("You're sure about that method name : %s %s\n",
			java_method_name, java_method_signature);

	// TODO Document this
	(*android.java_vm)->DetachCurrentThread(android.java_vm);
	
}

