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

#ifndef MYY_INCLUDE_FILE_HELPERS
#define MYY_INCLUDE_FILE_HELPERS 1

#include <stdint.h>

unsigned int fh_ReadFileToBuffer
(char const * restrict const name,
 void * restrict const buffer,
 const unsigned int size);

unsigned int fh_ReadFileToStringBuffer
(char const * restrict const name,
 void * restrict const buffer,
 const unsigned int size);

/** Copy the whole file contents into buffer, using the AssetsManager
 *
 * ASSUMPTIONS :
 * - There's a publicly available AssetsManager address identified by
 *   myy_assets_manager.
 * - 'buffer' can contain the whole file
 *
 * CAUTION :
 * If there's not enough space in buffer, this procedure will most
 * likely generate a SIGSEGV or, worse, corrupt the memory.
 *
 * PARAMS :
 * @param filepath The file's path in the Assets archive.
 * @param buffer   The buffer to store the file's content in.
 *
 * RETURNS :
 * 1 if the whole copy was done successfully
 * 0 otherwise
 */
unsigned int fh_WholeFileToBuffer
(char const * restrict const filepath,
 void * restrict const buffer);

#endif
