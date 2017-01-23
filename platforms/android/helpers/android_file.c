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

#include <myy/helpers/file.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <unistd.h> // SEEK_SET

extern AAssetManager *myy_assets_manager;

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
* @param pathname The file's path in the Assets archive.
* @param buffer   The buffer to store the file's content in.
*
* RETURNS :
* 1 if the whole copy was done successfully
* 0 otherwise
*/
unsigned int fh_WholeFileToBuffer
(const char * __restrict const pathname,
void * __restrict const buffer) {
	/* Couldn't they mount the whole archive as a filesystem ? */
	AAsset *file =
		AAssetManager_open(myy_assets_manager, pathname,
		                   AASSET_MODE_STREAMING);
	unsigned int file_size = AAsset_getLength(file);
	int read_bytes = AAsset_read(file, buffer, file_size);

	AAsset_close(file);
	return read_bytes == file_size;
}


int fh_ReadFileToBuffer
(const char * __restrict const pathname,
void * __restrict const buffer,
const unsigned int size) {
	AAsset *file =
		AAssetManager_open(myy_assets_manager, pathname,
		                   AASSET_MODE_STREAMING);

	int read_bytes = AAsset_read(file, buffer, size);

	AAsset_close(file);

	return read_bytes;
}

int fh_ReadFileToStringBuffer
(const char * __restrict const pathname,
void * __restrict const buffer,
const unsigned int size) {
	int read_bytes =
		fh_ReadFileToBuffer(pathname, buffer, size);
	if (read_bytes != -1)
		((uint8_t *) buffer)[read_bytes] = '\0';
	return read_bytes;
}

int fh_ReadBytesFromFile
(char const * __restrict const pathname,
 void * __restrict const buffer,
 unsigned int const size,
 unsigned int const offset)
{
	AAsset *file =
		AAssetManager_open(myy_assets_manager, pathname,
		                   AASSET_MODE_STREAMING);
	
	int read_bytes = -1;
	
	if (AAsset_seek(file, offset, SEEK_SET) == offset)
		read_bytes = AAsset_read(file, buffer, size);
	
	return read_bytes;

}
