#ifndef MYY_EVDEV
#define MYY_EVDEV 1

#include <libevdev/libevdev.h>

enum myy_evdev_handlers {
	myy_evdev_handler_event,
	myy_evdev_handler_missed_events,
	myy_evdev_n_handlers
};
struct myy_evdev_device {
	/* The opened device */
	struct libevdev *dev;
	void (*event_handlers[myy_evdev_n_handlers])
	(struct myy_evdev_device const * device,
	 struct input_event * event);
	int fd;
};

enum myy_evdev_dev_name {
  myy_evdev_mouse,
	myy_evdev_keyboard,
	myy_evdev_n_devices
};

struct myy_evdev_devices {
	struct myy_evdev_device device[myy_evdev_n_devices];
};

unsigned int myy_init_input_devices
(struct myy_evdev_devices * const devices);

unsigned int myy_free_input_devices
(struct myy_evdev_devices * const devices);

unsigned int myy_evdev_read_input
(struct myy_evdev_devices const * const devices);

#endif /* MYY_EVDEV */
