/*
  Copyright (c) 2017 Miouyouyou <Myy>

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

#include <myy/helpers/strings.h>
#include <myy/helpers/log.h>

/** Store the UTF-8 sequence corresponding to the provided UTF-32
 *  codepoint in the provided string.
 *
 * If you have an UTF-8 terminal, you can then just do :
 *   char string[5] = {0};
 *   utf32_to_utf8_string(L'真', string);
 *   printf("%s\n", string);
 *
 * WARNING: This assumes that you can store at least 4 bytes in
 *          the address identified by 'string'.
 *          This also assumes a little-endian system.
 *          Not tested on a Big Endian system.
 *
 * @param code   The UTF-32 codepoint to convert
 * @param string The byte array where the UTF-8 sequence will be
 *               stored
 */
void utf32_to_utf8_string(uint32_t code, char * string) {
	if (code < 0x80) string[0] = code;
	else if (code < 0x800) {   // 00000yyy yyxxxxxx
		string[0] = (0b11000000 | (code >> 6));
		string[1] = (0b10000000 | (code & 0x3f));
	}
	else if (code < 0x10000) {  // zzzzyyyy yyxxxxxx
		string[0] = (0b11100000 | (code >> 12));         // 1110zzz
		string[1] = (0b10000000 | ((code >> 6) & 0x3f)); // 10yyyyy
		string[2] = (0b10000000 | (code & 0x3f));        // 10xxxxx
	}
	else if (code < 0x200000) { // 000uuuuu zzzzyyyy yyxxxxxx
		string[0] = (0b11110000 | (code >> 18));          // 11110uuu
		string[1] = (0b10000000 | ((code >> 12) & 0x3f)); // 10uuzzzz
		string[2] = (0b10000000 | ((code >> 6)  & 0x3f)); // 10yyyyyy
		string[3] = (0b10000000 | (code & 0x3f));         // 10xxxxxx
	}
}

struct utf8_codepoint utf8_codepoint_and_size(uint8_t * string)
{
	uint8_t current_char = string[0];
	unsigned int codepoint = 0;
	unsigned int size = 1;

	unsigned int const multibyte_mask = 0b00111111;

	if (current_char < 0x80)
		codepoint = current_char;
	else if (current_char >> 5 == 0b110) {
		codepoint = (current_char & 0b00011111) << 6;
		codepoint |= (string[1] & multibyte_mask);
		size = 2;
	}
	else if (current_char >> 4 == 0b1110) {
		codepoint = (current_char & 0b00001111) << 12;
		codepoint |= (string[1] & multibyte_mask) << 6;
		codepoint |= (string[2] & multibyte_mask);
		size = 3;
	}
	else if (current_char >> 3 == 0b11110) {
		codepoint = (current_char & 0b00001111) << 18;
		codepoint |= (string[1] & multibyte_mask) << 12;
		codepoint |= (string[2] & multibyte_mask) << 6;
		codepoint |= (string[3] & multibyte_mask);
		size = 4;
	}
	else LOG("Malformed char : %d\n", current_char);

	struct utf8_codepoint converted_codepoint = {
		.raw = codepoint,
		.size = size
	};
	
	return converted_codepoint;
}
