/*
	Copyright (c) 2016 Miouyouyou <Myy>

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files 
	(the "Software"), to deal in the Software without restriction, 
  including	without limitation the rights to use, copy, modify, merge, 
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

/* I do not know how to inline this... It will stay as a macro until
   I figure out how the "inline" magic work */
#ifndef MYY_HELPERS_STRING_H
#define MYY_HELPERS_STRING_H 1
#define sh_pointToNextString(contiguous_strings) while (*contiguous_strings++)
#include <stdint.h>

unsigned int myy_string_size(const uint8_t * const string) {
  unsigned int c = 0;
  for(; string[c] != 0; c++);
  return c;
}

#endif
