#include <libevdev/libevdev.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <myy.h>
#include <myy/helpers/log.h>

#include <ftw.h>

#include <unistd.h>

#include "myy_evdev.h"

static void plus_x(int code, int value) {
	myy_rel_mouse_move(value, 0);
	// printf("X : %d\n", value);
}

static void plus_y(int code, int value) {
	myy_rel_mouse_move(0, -value);
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

static void parse_event
(struct input_event * __restrict const event) {
	/*printf("Event : %s (%d) %s (%d) %d\n",
	 libevdev_event_type_get_name(event->type),
	 event->type,
	 libevdev_event_code_get_name(event->type, event->code),
	 event->code,
	 event->value);*/
	if (event->type == EV_REL)
		plus_rels[event->code](event->code, event->value);
}

static void parse_dropped_events
(struct input_event * __restrict const event,
 struct libevdev * __restrict const dev) 
{
	int rc;
	printf("Resyncing !! ------------------------\n");
	do {
		parse_event(event);
		rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_SYNC, event);
	} while (rc == LIBEVDEV_READ_STATUS_SYNC);
	printf("Resync Complete ! ++++++++++++++++++\n");
}

void (*handlers[])() = {
	[LIBEVDEV_READ_STATUS_SUCCESS] = parse_event,
	[LIBEVDEV_READ_STATUS_SYNC] = parse_dropped_events,
};

static inline int is_a_valid_mouse
(struct libevdev const * const dev) {
  return (libevdev_has_event_type(dev, EV_REL) &&
          libevdev_has_event_code(dev, EV_KEY, BTN_LEFT));
}

static int mouse_check
(char const * __restrict const file_path, 
 struct stat const * __restrict const file_stats,
 int const filetype)
{
	struct libevdev *dev = NULL;
	int fd = open(file_path, O_RDONLY|O_NONBLOCK);
	int rc = libevdev_new_from_fd(fd, &dev);
	// nftw() continue if 0 is returned. 
	// Since we want to return the fd, we need to avoid 0 values
	int ret = fd + 1;

	if (rc < 0 || !is_a_valid_mouse(dev))
	{
		if (rc >= 0) close(fd);
		ret = 0;
	}
	libevdev_free(dev);
	return ret;
}

static int acquire_mouse() {
	int incremented_fd = 
		ftw("/dev/input", mouse_check, 1);
	int ret = -1;
	if (incremented_fd) ret = incremented_fd - 1;
	return ret;
}


unsigned int myy_evdev_read_input
(struct myy_evdev_data const * const mouse) 
{
	int rc = 0;
	int ret = 1;
    struct libevdev const * const dev = mouse->dev;
	
	do {
		struct input_event ev;
		rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
		if (rc >= 0) handlers[rc](&ev, dev);
	} while (rc == 0 || rc == 1);

	return 1;
}

static unsigned int init_mouse
(struct myy_evdev_data * const mouse) {

    int ret = 0;
	int fd = acquire_mouse();
	if (fd >= 0) {
		libevdev_new_from_fd(fd, &(mouse->dev));
		mouse->fd = fd;
		ret = 1;
	}
	return ret;
}

unsigned int myy_init_input_devices
(struct myy_evdev_data * const mouse,
 unsigned int const n_devices) 
{
	return init_mouse(mouse);
}

unsigned int myy_free_input_devices
(struct myy_evdev_data * const mouse,
 unsigned int const n_devices)
{
	libevdev_free(mouse->dev);
	close(mouse->fd);
}

// LIBEVDEV_READ_STATUS_SUCCESS 0
// LIBEVDEV_READ_STATUS_SYNC    1
