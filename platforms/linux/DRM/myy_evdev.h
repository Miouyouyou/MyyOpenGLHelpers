#ifndef MYY_EVDEV
#define MYY_EVDEV 1

#include <libevdev/libevdev.h>

struct myy_evdev_data {
	/* The opened device */
	struct libevdev *dev;
	int fd;
};

unsigned int myy_init_input_devices
(struct myy_evdev_data * const mouse,
 unsigned int const n_devices);

unsigned int myy_free_input_devices
(struct myy_evdev_data * const mouse,
 unsigned int const n_devices);

unsigned int myy_evdev_read_input
(struct myy_evdev_data const * const mouse);

#endif /* MYY_EVDEV */
