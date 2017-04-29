#include "init_window.h"

#include <stdlib.h>

void ParseEvents
(struct myy_xcb_state * __restrict const state)
{

	xcb_generic_event_t * event;

	while ((event = xcb_poll_for_event(state->connection))) {
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

				if (state->is_moving.button == 0) {
					state->is_moving.button = button;
				  state->is_moving.start_x = x;
					state->is_moving.start_y = y;
				}
				if (click_time - state->last_click > 250)
					myy_click(x, y, button);
				else myy_doubleclick(x, y, button);
				state->last_click = click_time;
			}
			break;
			case XCB_BUTTON_RELEASE: {
				state->is_moving.button = 0;
				break;
			}
			case XCB_MOTION_NOTIFY: {
				xcb_motion_notify_event_t * motion =
					(xcb_motion_notify_event_t *) event;
				
				if (state->is_moving.button == 0)
					myy_hover(motion->event_x, motion->event_y);
				else
					myy_move(
						motion->event_x, motion->event_y,
						state->is_moving.start_x, state->is_moving.start_y
					);
			}
			break;
			case XCB_KEY_PRESS: {
				// Keyboards values are shifted by 3 with X11, for
				// 'historical' reasons.
				xcb_key_press_event_t * kp =
					(xcb_key_press_event_t *) event;
				myy_key(kp->detail >> 3);
			}
			break;
			case XCB_KEY_RELEASE: {
				xcb_key_press_event_t * kp =
					(xcb_key_press_event_t *) event;
				myy_key_release(kp->detail >> 3);
			}
			break;
			case XCB_DESTROY_NOTIFY:
			case XCB_UNMAP_NOTIFY: {}
			break;
    }
    free(event);
  }

  return;
}
