#include <myy.h>
#include <myy/current/opengl.h>
#include <myy/helpers/macros.h>

void weak_function myy_user_quit(
	myy_states * __restrict const states) {
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

void weak_function myy_rel_mouse_move(
	myy_states * __restrict const states,
	int x, int y)
{}

void weak_function myy_mouse_action(
	myy_states * __restrict const states,
	enum mouse_action_type type,
	int value)
{
}

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
{}

void weak_function myy_stop(myy_states * __restrict const states) {}

void weak_function myy_click(
	myy_states * __restrict const states,
	int x, int y, unsigned int button)
{}

void weak_function myy_doubleclick(
	myy_states * __restrict const states,
	int x, int y, unsigned int button)
{}

void weak_function myy_move(
	myy_states * __restrict const states,
	int x, int y,
	int start_x, int start_y)
{}

void weak_function myy_hover(
	myy_states * __restrict const states, int x, int y)
{}

void weak_function myy_key(
	myy_states * __restrict const states,
	unsigned int keycode)
{}

void weak_function myy_key_release(
	myy_states * __restrict const states,
	unsigned int keycode)
{}

void weak_function myy_text(
	myy_states * __restrict const states,
	char const * __restrict const text,
	size_t const text_size)
{}

void weak_function myy_text_input_start(
	myy_states * __restrict state)
{}
void weak_function myy_text_input_stop(
	myy_states * __restrict state)
{}
