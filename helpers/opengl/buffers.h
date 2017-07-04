#ifndef MYY_HELPERS_OPENGL_BUFFERS_H
#define MYY_HELPERS_OPENGL_BUFFERS_H 1

#include <myy/current/opengl.h>
#include <myy/helpers/log.h>

typedef uint32_t gpu_buffer_size;
struct myy_gpu_buffer { 
	GLuint id, offset; 
	struct { gpu_buffer_size used, max; } size;
};
typedef struct myy_gpu_buffer gpu_buffer;

inline static void gpu_buffer_set
(gpu_buffer * __restrict const buffer,
 GLuint id, GLuint offset)
{
	buffer->id = id; buffer->offset = offset;
}

inline static void gpu_buffer_bind
(gpu_buffer const buffer)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer.id);
}

inline static gpu_buffer_size gpu_buffer_size_remaining
(gpu_buffer const buffer)
{
	return buffer.size.max - buffer.size.used;
}

inline static uint8_t gpu_buffer_enough_space_for_abs
(gpu_buffer const buffer, GLsizei amount, GLuint abs_offset)
{
	return buffer.size.max > ((abs_offset - buffer.offset) + amount);
}

inline static uint8_t gpu_buffer_enough_space_for_rel
(gpu_buffer const buffer, GLsizei amount, GLuint rel_offset)
{
	return buffer.size.max > (rel_offset + amount);
}

inline static void gpu_buffer_store
(gpu_buffer * __restrict const buffer, GLsizei size,
 void * __restrict const data)
{
	/*if (size < buffer->size.max)*/
		glBufferSubData(GL_ARRAY_BUFFER, buffer->offset, size, data);
	/*else {
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
		buffer->size.max = size;
	}*/
}

inline static void gpu_buffer_store_after
(gpu_buffer const buffer, GLsizei size, GLint offset, void * data)
{
	
	glBufferSubData(GL_ARRAY_BUFFER, buffer.offset + offset, size, data);
}

inline static void gpu_buffer_store_at
(GLsizei size, GLint offset, void * data)
{
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}

/**
 * gpu_3buffs_dumb represents a dumb Triple Buffering mechanism, 
 * where buffers are only updated by regenerating them completely 
 * every time.
 * 
 * Given its simplicity, it does not store any update log and
 * cannot ensure synchronisation of the three buffers.
 * Still, store functions will resize the GPU buffers when the
 * currently allocated size is below the data size.
 * 
 */

struct gpu_triple_buffers_dumb {
	GLuint id[3];
	int16_t points[3];
	uint16_t current_index;
	struct { int32_t max[3]; } size;
};
typedef struct gpu_triple_buffers_dumb gpu_dumb_3buffs_t;

/**
 * gpu_dumb_3buffs_current_buffer_id
 *
 * Arguments :
 * The triple buffer structure
 *
 * Get the currently bound OpenGL buffer ID
 */
inline static GLuint gpu_dumb_3buffs_current_buffer_id
(gpu_dumb_3buffs_t const * __restrict const buffer)
{
	return buffer->id[buffer->current_index];
}

/**
 * gpu_dumb_3buffs_init
 * 
 * Generate 3 buffers identifiers, bind them as GL_ARRAY_BUFFER
 * targets and prepare, for each buffer, a storage space in the GPU 
 * equivalent to 'original_size'.
 * That means that the original_size * 3 bytes will be allocated in
 * the GPU memory !
 * These buffers usage will be set to 'usage'.
 * 
 * Parameters :
 * @param buffer The triple buffer structure to initialize.
 * @param original_size The original size.
 * @param usage The OpenGL usage hint for these buffers.
 *
 * Example :
 * gpu_dumb_3buffs_t buffers;
 * gpu_dumb_3buffs_init(&buffers, 8192, GL_DYNAMIC_DRAW);
 * (Allocate 24Kib (8192 bytes * 3) of memory space in the GPU)
 */
void gpu_dumb_3buffs_init
(gpu_dumb_3buffs_t * __restrict const buffer,
 uint32_t const original_size,
 GLenum const usage);

/**
 * gpu_dumb_3buffs_bind
 *
 * PARAMS:
 * @param buffer The triple buffer structure providing the ID to bind.
 * 
 * Bind the current buffer ID of 'buffer' as a GL_ARRAY_BUFFER target.
 */
inline static void gpu_dumb_3buffs_bind
(gpu_dumb_3buffs_t const * __restrict const buffer)
{
	glBindBuffer(
		GL_ARRAY_BUFFER, gpu_dumb_3buffs_current_buffer_id(buffer)
	);
}

/**
 * gpu_dumb_3buffs_bind_next_buffer()
 *
 * Binds the next buffer in a round-robin manner.
 *
 * PARAMS:
 * @param buffer The triple buffer structure to get the next buffer ID
 *               from.
 */
inline static void gpu_dumb_3buffs_bind_next_buffer
(gpu_dumb_3buffs_t * __restrict const buffer)
{
	uint8_t current_id = buffer->current_index;
	uint8_t new_index = (current_id != 2) ? current_id + 1 : 0;
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer->id[new_index]);
	buffer->current_index = new_index;
}

/**
 * gpu_dumb_3buffs_store
 *
 * Store size bytes of the provided data into the currently bound
 * OpenGL buffer, starting at offset 0, using glBufferSubData.
 * If the current buffer ID cannot store that much bytes, the buffer
 * is resized to twice the number of bytes required (2 * size),
 * using glBufferData.
 *
 * WARNING:
 * This does *NOT* rebind the current buffer ID, nor check if the
 * currently bound ID IS equal to gpu_dumb_3buffs_current_buffer_id().
 * These checks must be performed by the caller !
 *
 * PARAMS:
 * @param buffer The triple buffer structure that will only be used
 *               if the current.
 * @param size The number of bytes to store from the provided 'data'.
 * @param data The data to store in the currently bound OpenGL buffer.
 */
void gpu_dumb_3buffs_store
(gpu_dumb_3buffs_t * __restrict const buffer,
 uint32_t const size, void * __restrict const data);

#endif
