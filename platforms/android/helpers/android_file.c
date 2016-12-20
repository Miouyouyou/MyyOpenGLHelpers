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
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

extern AAssetManager *myy_assets_manager;

unsigned int fh_WholeFileToBuffer
(const char * restrict const filepath,
 void * restrict const buffer) {
  /* Couldn't they mount the whole archive as a filesystem ? */
  AAsset *file =
    AAssetManager_open(myy_assets_manager, filepath,
                       AASSET_MODE_STREAMING);
  unsigned int
    file_size = AAsset_getLength(file),
    read_bytes = AAsset_read(file, buffer, file_size);

  AAsset_close(file);
  return read_bytes == file_size;
}


unsigned int fh_ReadFileToBuffer
(const char * restrict const filepath,
 void * restrict const buffer,
 const unsigned int size) {
  AAsset *file =
    AAssetManager_open(myy_assets_manager, filepath,
                       AASSET_MODE_STREAMING);

  unsigned int
    read_bytes = AAsset_read(file, buffer, size);

  AAsset_close(file);

  return read_bytes;
}

unsigned int fh_ReadFileToStringBuffer
(const char * restrict const filepath,
 void * restrict const buffer,
 const unsigned int size) {
  unsigned int read_bytes =
    fh_ReadFileToBuffer(filepath, buffer, size);
  ((uint8_t *) buffer)[read_bytes] = '\0';
  return read_bytes;
}
