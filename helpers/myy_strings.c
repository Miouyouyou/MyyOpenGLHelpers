#include <myy/helpers/string.h>

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

