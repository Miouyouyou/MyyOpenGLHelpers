#include <myy.h>
#include <myy/current/opengl.h>
#include <myy/helpers/macros.h>

void weak_function myy_user_quit(
	myy_states * __restrict const states) {
	LOG("User quit\n");
	myy_platform_stop(states);
}

int weak_function myy_init(
	myy_states * __restrict const states,
	int argc,
	char **argv,
	struct myy_window_parameters * __restrict const parameters)
{
	return 0;
};

void weak_function myy_display_initialised(
	myy_states * __restrict const states,
	unsigned int const width,
	unsigned int const height)
{}

void weak_function myy_generate_new_state(
	myy_states * __restrict const states)
{}

void weak_function myy_init_drawing(
	myy_states * __restrict const states,
	uintreg_t surface_width,
	uintreg_t surface_height)
{}

void weak_function myy_draw_before(
	myy_states * __restrict const states,
	uintreg_t i,
	uint64_t last_frame_delta_ns)
{}

void weak_function myy_draw(
	myy_states * __restrict const states,
	uintreg_t i,
	uint64_t last_frame_delta_ns)
{}

void weak_function myy_draw_after(
	myy_states * __restrict const states,
	uintreg_t i,
	uint64_t last_frame_delta_ns) 
{}

void weak_function myy_save_state(
	myy_states * __restrict const states,
	void * const save_state)
{}

void weak_function myy_resume_state(
	myy_states * __restrict const states,
	void * const save_state)
{}

void weak_function myy_cleanup_drawing(
	myy_states * __restrict const states)
{ LOG("Cleanup drawing.\n"); }

void weak_function myy_stop(myy_states * __restrict const states) { LOG("Stopping...\n"); }

void weak_function myy_text_input_start(
	myy_states * __restrict state)
{}
void weak_function myy_text_input_stop(
	myy_states * __restrict state)
{}


void weak_function myy_input(
	myy_states * __restrict state,
	enum myy_input_events const event_type,
	union myy_input_event_data * __restrict const event)
{
	switch(event_type) {
		case myy_input_event_invalid:
		case myy_input_event_mouse_moved_absolute:
		case myy_input_event_mouse_moved_relative:
		case myy_input_event_mouse_button_pressed:
		case myy_input_event_mouse_button_released:
		case myy_input_event_touch_pressed:
		case myy_input_event_touch_released:
		case myy_input_event_keyboard_key_released:
			break;
		case myy_input_event_keyboard_key_pressed:
			LOG("KEY: %d\n", event->key.raw_code);
			if (event->key.raw_code == 1) { myy_user_quit(state); }
			break;
		case myy_input_event_text_received:
			LOG("TEXT: %s\n", event->text.data);
			break;
		case myy_input_event_editor_finished:
			LOG("EDITOR: %s\n", event->text.data);
			break;
		case myy_input_event_surface_size_changed:
			myy_display_initialised(
				state, event->surface.width, event->surface.height);
			break;
		case myy_input_event_window_destroyed:
			myy_user_quit(state);
			break;
		default:
			break;
	}
}
