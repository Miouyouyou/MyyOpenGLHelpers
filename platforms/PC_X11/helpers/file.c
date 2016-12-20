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

#include <helpers/file.h>

/* read - close */
#include <unistd.h>

/* open */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

unsigned int fh_WholeFileToBuffer
(const char * restrict const filepath,
 void * restrict const buffer) {

  ssize_t bytes_read;
  off_t file_size;
  struct stat fd_stats;

  int fd = open(filepath, O_RDONLY);

  if (fd != -1) {

    fstat(fd, &fd_stats);

    file_size = fd_stats.st_size;

    bytes_read = read(fd, buffer, file_size);
    close(fd);

    return bytes_read == file_size;

  }
  else return 0;
}

unsigned int fh_ReadFileToBuffer
(const char * restrict const name, 
 void * restrict const buffer, 
 const unsigned int size) {

  int fd = open(name, O_RDONLY);
  int bytes_read = read(fd, buffer, size);
  close(fd);
  return bytes_read;

}

unsigned int fh_ReadFileToStringBuffer
(const char * restrict const name, 
 void * restrict const buffer, 
 const unsigned int size) {

  unsigned int bytes_read = fh_ReadFileToBuffer(name, buffer, size);
  ((uint8_t *) buffer)[bytes_read] = 0;
  return bytes_read;

}

