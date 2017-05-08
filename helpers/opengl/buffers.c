#include <myy/helpers/opengl/buffers.h>

void gpu_dumb_3buffs_init
(gpu_dumb_3buffs_t * __restrict const buffer,
 uint32_t const original_size, GLenum const usage)
{
	glGenBuffers(3, buffer->id);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->id[0]);
	glBufferData(GL_ARRAY_BUFFER, original_size, NULL, usage);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->id[1]);
	glBufferData(GL_ARRAY_BUFFER, original_size, NULL, usage);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->id[2]);
	glBufferData(GL_ARRAY_BUFFER, original_size, NULL, usage);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->id[0]);

	buffer->current_index = 0;
	buffer->size.max[2] =
		buffer->size.max[1] =
		buffer->size.max[0] =
		original_size;
}

void gpu_dumb_3buffs_store
(gpu_dumb_3buffs_t * __restrict const buffer,
 uint32_t const size, void * __restrict const data)
{

	uint8_t current_index = current_index = buffer->current_index;
	if (size < buffer->size.max[current_index])
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	else {
		uint32_t new_size = size * 2;
		glBufferData(GL_ARRAY_BUFFER, new_size, data, GL_DYNAMIC_DRAW);
		buffer->size.max[current_index] = new_size;
	}
}
