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
#include <string.h>
#include <myy/helpers/log.h>

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

struct myy_fh_map_handle fh_MapFileToMemory
(char const * __restrict const pathname)
{
	
	struct myy_fh_map_handle handle = {
		.ok = 0,
		.address = NULL,
		.length = 0
	};
	
	AAsset * file = AAssetManager_open(
		myy_assets_manager, pathname, AASSET_MODE_STREAMING
	);
	
	if (file != NULL) {
		void * __restrict const address = AAsset_getBuffer(file);
		int asset_length = AAsset_getLength(file);
		handle.ok = 1;
		handle.address = address;
		handle.length = asset_length;
		handle.id = (intptr_t) file;
	}
	
	return handle;
}

void fh_UnmapFileFromMemory
(struct myy_fh_map_handle const handle)
{
	AAsset_close((AAsset *) handle.id);
}

struct myy_fh_map_handle myy_asset_map_to_memory(
	char const * __restrict const rel_filepath,
	enum myy_asset_type const type)
{
	char complete_filepath[256] = {0};

	struct myy_fh_map_handle handle = {
		.ok = 0,
		.address = NULL,
		.length = 0
	};

	switch(type) {
		case myy_asset_type_texture:
		{
			/* We don't care about the trailing \0 of the prefix */
			size_t const prefix_size = sizeof("textures/")-1;
			/* Take account for the leading '\0' */
			size_t const max_allowed_size =
				sizeof(complete_filepath) - prefix_size - 1;
			size_t provided_filepath_length =
				strnlen(rel_filepath, max_allowed_size);
			if (provided_filepath_length < max_allowed_size) {
				strncpy(
					complete_filepath,
					"textures/",
					prefix_size);
				strncpy(
					complete_filepath+prefix_size,
					rel_filepath,
					provided_filepath_length);
				LOG("Loading texture : %s\n", complete_filepath);
				handle = fh_MapFileToMemory(complete_filepath);
			}
		}
		break;
		default: break;
	}
	return handle;
}
