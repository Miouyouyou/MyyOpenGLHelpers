#include <libevdev/libevdev.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <myy/myy.h>
#include <myy/helpers/log.h>

#include <ftw.h>

#include <unistd.h>

#include "myy_evdev.h"

static void plus_x(int code, int value) {
	myy_rel_mouse_move(value, 0);
	// printf("X : %d\n", value);
}

static void plus_y(int code, int value) {
	myy_rel_mouse_move(0, value);
}

static void plus_wheel(int code, int value) {
	myy_mouse_action(myy_mouse_wheel_action, value);
}

static void plus_wut(int code, int value) {
	LOG("??? : %d\n", value);
}

static void (*plus_rels[])(int code, int value) = {
	[REL_X] = plus_x,
	[REL_Y] = plus_y,
	[REL_WHEEL] = plus_wheel,
	plus_wut
};

static void parse_mouse_event
(struct myy_evdev_device const * __restrict const device,
 struct input_event * __restrict const event) {
	if (event->type == EV_REL)
		plus_rels[event->code](event->code, event->value);
}

static void parse_keyboard_event
(struct myy_evdev_device const * __restrict const device,
 struct input_event * __restrict const event) {
 	if (event->type == EV_KEY) myy_key(event->code);
	fprintf(
		stderr,
		"[parse_keyboard_event]\n"
		"  event_type : %s (%d) - event_code - %s\n (%d)",
		libevdev_event_type_get_name(event->type),
		event->type,
		libevdev_event_code_get_name(event->type, event->code),
		event->code
	);
}

static void parse_dropped_events
(struct myy_evdev_device const * __restrict const device,
 struct input_event * __restrict const event)
{
	int rc;
	fprintf(stderr, "Resyncing !! ------------------------\n");
	do {
		device->event_handlers[myy_evdev_handler_event](device, event);
		rc = libevdev_next_event(device->dev, LIBEVDEV_READ_FLAG_SYNC, event);
	} while (rc == LIBEVDEV_READ_STATUS_SYNC);
	fprintf(stderr, "Resync Complete ! ++++++++++++++++++\n");
}

static inline int is_a_valid_mouse
(struct libevdev const * const dev) {
	return (
		libevdev_has_event_type(dev, EV_REL) &&
		libevdev_has_event_code(dev, EV_KEY, BTN_LEFT)
	);
}

static inline int is_a_valid_keyboard
(struct libevdev const * const dev) {
	return (
		libevdev_has_event_type(dev, EV_KEY) &&
		libevdev_has_event_code(dev, EV_KEY, KEY_ENTER)
	);
}

static int is_a_valid_device
(char const * __restrict const file_path,
 int (*device_valid_check)(struct libevdev const *))
{
	struct libevdev *dev = NULL;
	int fd = open(file_path, O_RDONLY|O_NONBLOCK);
	int rc = libevdev_new_from_fd(fd, &dev);
	// nftw() continue if 0 is returned. 
	// Since we want to return the fd, we need to avoid 0 values
	int ret = fd + 1;

	if (rc < 0 || !device_valid_check(dev))	{
		if (rc >= 0) close(fd);
		ret = 0;
	}
	libevdev_free(dev);
	return ret;

}

static int keyboard_check
(char const * __restrict const file_path,
 struct stat const * __restrict const file_stats,
 int const filetype)
{
	int check = is_a_valid_device(file_path, is_a_valid_keyboard);
	LOG("%s is a valid keyboard : %d\n", file_path, check);
	return check;
}

static int mouse_check
(char const * __restrict const file_path, 
 struct stat const * __restrict const file_stats,
 int const filetype)
{
	int check = is_a_valid_device(file_path, is_a_valid_keyboard);
	LOG("%s is a valid mouse : %d\n", file_path, check);
	return check;
}

static int acquire_device
(int (*check_func)(char const *, struct stat const *, int))
{
	int incremented_fd = ftw("/dev/input", check_func, 1);
	int ret = -1;
	if (incremented_fd) ret = incremented_fd - 1;
	return ret;
}

static unsigned int init_device
(struct myy_evdev_device * __restrict const device,
 int (*check_func)(char const *, struct stat const *, int),
 void (*event_handler)
   (struct myy_evdev_device const *, struct input_event *))
{

	int ret = 0;
	int fd = acquire_device(check_func);
	if (fd >= 0) {
		libevdev_new_from_fd(fd, &(device->dev));
		device->event_handlers[myy_evdev_handler_event] =
			event_handler;
		device->event_handlers[myy_evdev_handler_missed_events] =
			parse_dropped_events;
		ret = 1;
	}
	device->fd = fd;
	return ret;
}

unsigned int myy_evdev_read_input
(struct myy_evdev_devices const * const devices) 
{
	for (
		enum myy_evdev_dev_name id = myy_evdev_mouse;
		id < myy_evdev_n_devices; id++
	)
	{
		int rc = 0;
		int ret = 1;
		struct myy_evdev_device const * const device =
			devices->device+id;
	
		do {
			struct input_event ev;
			rc = libevdev_next_event(
				device->dev, LIBEVDEV_READ_FLAG_NORMAL, &ev
			);
			if (rc >= 0) device->event_handlers[rc](device, &ev);
		} while (rc == 0 || rc == 1);
	}

	return 1;
}

unsigned int myy_init_input_devices
(struct myy_evdev_devices * const devices) 
{
	unsigned int status = init_device(
		devices->device+myy_evdev_mouse, mouse_check, parse_mouse_event
	);
	status &= init_device(
		devices->device+myy_evdev_keyboard, keyboard_check, parse_keyboard_event
	);
	return status;
}

unsigned int myy_free_input_devices
(struct myy_evdev_devices * const devices)
{
	for (
		enum myy_evdev_dev_name d = myy_evdev_mouse;
		d < myy_evdev_n_devices; d++
	) {
		if (devices->device[d].fd != -1) {
			libevdev_free(devices->device[d].dev);
			close(devices->device[d].fd);
		}
	}
	return 1;
}

// LIBEVDEV_READ_STATUS_SUCCESS 0
// LIBEVDEV_READ_STATUS_SYNC    1
