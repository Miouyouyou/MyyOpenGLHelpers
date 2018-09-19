#include <myy.h>
#include <myy/current/opengl.h>
#include <myy/helpers/macros.h>

void weak_function myy_init() {};

void weak_function myy_display_initialised(
	unsigned int const width,
	unsigned int const height)
{
}

void weak_function myy_generate_new_state() {}

void weak_function myy_init_drawing() {}

void weak_function myy_draw() {

}

void weak_function myy_after_draw() {}

void weak_function myy_rel_mouse_move(int x, int y) {
}

void weak_function myy_mouse_action(
	enum mouse_action_type type,
	int value)
{
}

void weak_function myy_save_state(
	struct myy_game_state * const state)
{}

void weak_function myy_resume_state(
	struct myy_game_state * const state)
{}

void weak_function myy_cleanup_drawing() {}

void weak_function myy_stop() {}

void weak_function myy_click(int x, int y, unsigned int button) {}

void weak_function myy_doubleclick(int x, int y, unsigned int button)
{}

void weak_function myy_move(int x, int y, int start_x, int start_y)
{}

void weak_function myy_hover(int x, int y) {}

void weak_function myy_key(unsigned int keycode)
{
}

void weak_function myy_key_release(unsigned int keycode)
{}

