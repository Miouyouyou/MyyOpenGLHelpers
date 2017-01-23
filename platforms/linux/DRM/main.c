/*
 * Copyright (c) 2012 Arvin Schnell <arvin.schnell@gmail.com>
 * Copyright (c) 2012 Rob Clark <rob@ti.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* Based on a egl cube test app originally written by Arvin Schnell */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <assert.h>

#include <myy.h>
#include <myy/egl_common/myy_eglattrs.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


struct egl_infos {
	EGLDisplay display;
	EGLConfig config;
	EGLContext context;
	EGLSurface surface;
};

struct gbm_infos {
	struct gbm_device *dev;
	struct gbm_surface *surface;
};

struct drm_infos {
	int fd;
	drmModeModeInfo *mode;
	uint32_t crtc_id;
	uint32_t connector_id;
};

struct drm_fb {
	struct gbm_bo *bo;
	struct drm_infos *drm;
	uint32_t fb_id;
};

static uint32_t find_crtc_for_encoder(const drmModeRes *resources,
				      const drmModeEncoder *encoder) {
	int i;

	for (i = 0; i < resources->count_crtcs; i++) {
		/* possible_crtcs is a bitmask as described here:
		 * https://dvdhrm.wordpress.com/2012/09/13/linux-drm-mode-setting-api
		 */
		const uint32_t crtc_mask = 1 << i;
		const uint32_t crtc_id = resources->crtcs[i];
		if (encoder->possible_crtcs & crtc_mask) {
			return crtc_id;
		}
	}

	/* no match found */
	return -1;
}

static uint32_t find_crtc_for_connector
(drmModeRes const * __restrict const resources,
 drmModeConnector const * __restrict const connector,
 struct drm_infos const * __restrict const drm_infos)
{
	int i;

	for (i = 0; i < connector->count_encoders; i++) {
		const uint32_t encoder_id = connector->encoders[i];
		drmModeEncoder *encoder =
		  drmModeGetEncoder(drm_infos->fd, encoder_id);

		if (encoder) {
			const uint32_t crtc_id =
			  find_crtc_for_encoder(resources, encoder);

			drmModeFreeEncoder(encoder);
			if (crtc_id != 0) {
				return crtc_id;
			}
		}
	}

	/* no match found */
	return -1;
}

static int init_drm
(struct drm_infos * const drm_infos)
{
	drmModeRes *resources;
	drmModeConnector *connector = NULL;
	drmModeEncoder *encoder = NULL;
	drmModeModeInfo *mode;
	int i, area, fd;
	uint32_t crtc_id;

	fd = open("/dev/dri/card0", O_RDWR);

	if (fd < 0) {
		printf("could not open drm device\n");
		return -1;
	}

	resources = drmModeGetResources(fd);
	if (!resources) {
		printf("drmModeGetResources failed: %s\n", strerror(errno));
		return -1;
	}

	/* find a connected connector: */
	printf("Connectors : %d\n", resources->count_connectors);
	for (i = 0; i < resources->count_connectors; i++) {
		connector = drmModeGetConnector(
		  fd, resources->connectors[i]
		);
		if (connector->connection == DRM_MODE_CONNECTED) {
			/* it's connected, let's use this! */
			break;
		}
		drmModeFreeConnector(connector);
		connector = NULL;
	}

	if (!connector) {
		/* we could be fancy and listen for hotplug events and wait for
		 * a connector..
		 */
		printf("no connected connector!\n");
		return -1;
	}

	/* find prefered mode or the highest resolution mode: */
	for (i = 0, area = 0; i < connector->count_modes; i++) {
		drmModeModeInfo *current_mode = &connector->modes[i];

		if (current_mode->type & DRM_MODE_TYPE_PREFERRED) {
			mode = current_mode;
		}

		int current_area = current_mode->hdisplay * current_mode->vdisplay;
		if (current_area > area) {
			mode = current_mode;
			area = current_area;
		}
	}

	if (!mode) {
		printf("could not find mode!\n");
		return -1;
	}

	/* find encoder: */
	for (i = 0; i < resources->count_encoders; i++) {
		encoder = drmModeGetEncoder(fd, resources->encoders[i]);
		if (encoder->encoder_id == connector->encoder_id)
			break;
		drmModeFreeEncoder(encoder);
		encoder = NULL;
	}

	if (encoder) {
		crtc_id = encoder->crtc_id;
	} else {
		uint32_t crtc_id = find_crtc_for_connector(
		  resources, connector, drm_infos
		);
		if (crtc_id == 0) {
			printf("no crtc found!\n");
			return -1;
		}

		crtc_id = crtc_id;
	}

	drm_infos->fd = fd;
	drm_infos->mode = mode;
	drm_infos->connector_id = connector->connector_id;
	drm_infos->crtc_id = crtc_id;
	return 0;
}

static int init_gbm
(struct drm_infos * __restrict const drm_infos,
 struct gbm_infos * __restrict const gbm_infos)
{
	gbm_infos->dev = gbm_create_device(drm_infos->fd);

	gbm_infos->surface = gbm_surface_create(
	  gbm_infos->dev,
	  drm_infos->mode->hdisplay, drm_infos->mode->vdisplay,
	  GBM_FORMAT_ARGB8888, 
	  GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING
	);

	int ret = 0;
	if (!gbm_infos->surface) {
		printf("failed to create gbm surface\n");
		ret = -1;
	}

	return ret;
}

static int init_gl
(struct egl_infos * const egl_infos,
 struct gbm_infos * const gbm_infos)
{
	EGLint major, minor, n;
	GLuint vertex_shader, fragment_shader;
	GLint ret;

	EGLDisplay display;
	EGLConfig config;
	EGLContext context;
	EGLSurface surface;

	static const EGLint context_attribs[] = {
		MYY_CURRENT_GL_CONTEXT,
		EGL_NONE, EGL_NONE
	};

	static const EGLint config_attribs[] = {
		MYY_EGL_COMMON_PC_ATTRIBS,
		EGL_NONE, EGL_NONE
	};

	PFNEGLGETPLATFORMDISPLAYEXTPROC get_platform_display = NULL;
	get_platform_display =
		(void *) eglGetProcAddress("eglGetPlatformDisplayEXT");
	assert(get_platform_display != NULL);

	display =
	  get_platform_display(EGL_PLATFORM_GBM_KHR, gbm_infos->dev, NULL);

	if (!eglInitialize(display, &major, &minor)) {
		printf("failed to initialize\n");
		return -1;
	}

	printf("Using display %p with EGL version %d.%d\n",
			display, major, minor);

	printf("EGL Version \"%s\"\n", eglQueryString(display, EGL_VERSION));
	printf("EGL Vendor \"%s\"\n", eglQueryString(display, EGL_VENDOR));
	printf("EGL Extensions \"%s\"\n", eglQueryString(display, EGL_EXTENSIONS));

	if (!eglBindAPI(EGL_OPENGL_ES_API)) {
		printf("failed to bind api EGL_OPENGL_ES_API\n");
		return -1;
	}

	if (!eglChooseConfig(display, config_attribs, &config, 1, &n) || n != 1) {
		printf("failed to choose config: %d\n", n);
		return -1;
	}

	context = eglCreateContext(
	  display, config, EGL_NO_CONTEXT, context_attribs
	);
	if (context == NULL) {
		printf("failed to create context\n");
		return -1;
	}

	surface = eglCreateWindowSurface(
	  display, config, gbm_infos->surface, NULL
	);
	if (surface == EGL_NO_SURFACE) {
		printf("failed to create egl surface\n");
		return -1;
	}

	/* connect the context to the surface */
	eglMakeCurrent(display, surface, surface, context);

	printf("GL Extensions: \"%s\"\n", glGetString(GL_EXTENSIONS));
	egl_infos->display = display;
	egl_infos->config  = config;
	egl_infos->surface = surface;
	egl_infos->context = context;

	return 0;
}

static void
drm_fb_destroy_callback(struct gbm_bo *bo, void *data)
{
	myy_cleanup_drawing();
	struct drm_fb *fb = data;
	struct gbm_device *gbm = gbm_bo_get_device(bo);

	if (fb->fb_id)
		drmModeRmFB(fb->drm->fd, fb->fb_id);

	free(fb);
}

static struct drm_fb * drm_fb_get_from_bo
(struct gbm_bo * __restrict const bo, 
 struct drm_infos * __restrict const drm_infos)
{
	struct drm_fb *fb = gbm_bo_get_user_data(bo);
	uint32_t width, height, stride, handle;
	int ret;

	if (fb)
		return fb;

	fb = calloc(1, sizeof *fb);
	fb->bo = bo;
	fb->drm = drm_infos;

	width = gbm_bo_get_width(bo);
	height = gbm_bo_get_height(bo);
	stride = gbm_bo_get_stride(bo);
	handle = gbm_bo_get_handle(bo).u32;

	ret = drmModeAddFB(
	  drm_infos->fd,
	  width, height,
	  24, 32,
	  stride, handle, &fb->fb_id
	);
	if (ret) {
		printf("failed to create fb: %s\n", strerror(errno));
		free(fb);
		return NULL;
	}

	gbm_bo_set_user_data(bo, fb, drm_fb_destroy_callback);

	return fb;
}

static void page_flip_handler
(int fd, unsigned int frame,
 unsigned int sec, unsigned int usec,
 void * data)
{
	int *waiting_for_flip = data;
	*waiting_for_flip = 0;
}

int main(int argc, char *argv[])
{
	struct egl_infos egl;
	struct gbm_infos gbm;
	struct drm_infos drm;
	fd_set fds;
	drmEventContext evctx = {
		.version = DRM_EVENT_CONTEXT_VERSION,
		.page_flip_handler = page_flip_handler,
	};
	struct gbm_bo *bo;
	struct drm_fb *fb;
	uint32_t i = 0;
	int ret;

	ret = init_drm(&drm);
	if (ret) {
		printf("failed to initialize DRM\n");
		goto program_end;
	}

	FD_ZERO(&fds);
	FD_SET(0, &fds);
	FD_SET(drm.fd, &fds);

	ret = init_gbm(&drm, &gbm);
	if (ret) {
		printf("failed to initialize GBM\n");
		goto program_end;
	}

	ret = init_gl(&egl, &gbm);
	if (ret) {
		printf("failed to initialize EGL\n");
		goto program_end;
	}

	/* clear the color buffer */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	eglSwapBuffers(egl.display, egl.surface);
	bo = gbm_surface_lock_front_buffer(gbm.surface);
	fb = drm_fb_get_from_bo(bo, &drm);

	/* set mode: */
	ret = drmModeSetCrtc(drm.fd, drm.crtc_id, fb->fb_id, 0, 0,
			&drm.connector_id, 1, drm.mode);
	if (ret) {
		printf("failed to set mode: %s\n", strerror(errno));
		goto program_end;
	}

	myy_generate_new_state();
	myy_init_drawing();
	while (1) {
		struct gbm_bo *next_bo;
		int waiting_for_flip = 1;

		myy_draw();

		eglSwapBuffers(egl.display, egl.surface);
		next_bo = gbm_surface_lock_front_buffer(gbm.surface);
		fb = drm_fb_get_from_bo(next_bo, &drm);

		/*
		 * Here you could also update drm plane layers if you want
		 * hw composition
		 */

		ret = drmModePageFlip(drm.fd, drm.crtc_id, fb->fb_id,
				DRM_MODE_PAGE_FLIP_EVENT, &waiting_for_flip);
		if (ret) {
			printf("failed to queue page flip: %s\n", strerror(errno));
			ret = -1;
			goto program_end;
		}

		while (waiting_for_flip) {
			ret = select(drm.fd + 1, &fds, NULL, NULL, NULL);
			if (ret < 0) {
				printf("select err: %s\n", strerror(errno));
				goto program_end;
			} else if (ret == 0) {
				printf("select timeout!\n");
				ret = -1;
				goto program_end;
			} else if (FD_ISSET(0, &fds)) {
				printf("user interrupted!\n");
				break;
			}
			drmHandleEvent(drm.fd, &evctx);
		}

		/* release last buffer to render on again: */
		gbm_surface_release_buffer(gbm.surface, bo);
		bo = next_bo;
	}

program_end:
	return ret;
}
