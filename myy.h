/*
  Copyright (c) 2016 Miouyouyou <Myy>

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

#ifndef MYY_INCLUDED
#define MYY_INCLUDED 1

#include <stdint.h>

struct myy_game_state {
  unsigned int saved;
  unsigned int size;
  uint8_t state[228];
};

void myy_display_initialised(unsigned int width, unsigned int height);
void myy_init();
void myy_init_drawing();
void myy_draw();
void myy_cleanup_drawing();
void myy_stop();

void myy_generate_new_state();
void myy_save_state(struct myy_game_state *state);
void myy_resume_state(struct myy_game_state *state);

void myy_click(int x, int y, unsigned int button);
void myy_doubleclick(int x, int y, unsigned int button);
void myy_hover(int x, int y);
void myy_move(int x, int y);
void myy_key(unsigned int keycode);


#endif 
