#include <myy/current/opengl.h>

namespace Myy {
	namespace GL {
		/**
		 * Myy::GL::TripleSimpleBuffers represent a simple Triple Buffering
		 * mechanism, where buffers are only updated by regenerating them
		 * completely each time.
		 * 
		 * Given its simplicity, it does not store any update log and
		 * cannot ensure synchronisation of the three buffers.
		 * Still, store functions will resize the GPU buffers when the
		 * currently allocated size is below the data size.
		 * 
		 * There is no special constructor on this data structure, in order
		 * to keep it as a POD structure.
		 * 
		 */
		struct TripleSimpleBuffers {
			// Data
			GLuint id[3];
			int16_t points[3];
			uint16_t current_index;
			struct { int32_t max[3]; } size;
			
			// Constructors
			
			// Inline functions
			inline GLuint current_buffer_id() const
			{
				return id[current_index];
			}
			
			/**
			 * Myy::GL::TripleSimpleBuffers#bind()
			 * 
			 * Bind the current buffer ID as a GL_ARRAY_BUFFER target.
			 */
			inline static void bind() const
			{
				glBindBuffer(GL_ARRAY_BUFFER, current_buffer_id());
			}

			// Functions
		 /**
		  * Myy::GL::TripleSimpleBuffers#initialize(uint32_t original_size)
		  * Generate 3 buffers identifiers, bind them as GL_ARRAY_BUFFER
		  * targets and prepare, for each buffer, a storage space in the GPU 
		  * equivalent to 'original_size'.
		  * That means that the original_size * 3 bytes will be allocated in
		  * the GPU memory !
			* These buffers usage will be set to GL_DYNAMIC_DRAW.
			* (Because if you're aiming for a GL_STATIC_DRAW usage, the triple
			*  buffer mechanism does not make a lot of sense. You can easily
			*  incur the cost of buffer ghosting, or the complete drain of the
			*  draw pipeline for very rare updates.)
		  * 
		  * Example :
		  * auto buffers = TripleSimpleBuffers();
		  * buffers.initialize(8192);
		  * (Allocate 24Kib (8192 bytes * 3) of memory space in the GPU)
		  */
			void initialize(uint32_t original_size);
			
			/**
			 * Myy::GL::TripleSimpleBuffers#bind_next_buffer()
			 * Switch to the next buffer ID and bind it as a GL_ARRAY_BUFFER
			 * target.
			 * 
			 * Mainly used before sending new data to this triple buffer.
			 */
			void bind_next_buffer();
			
			/**
			 * Myy::GL::TripleSimpleBuffers#store
			 * (uint32_t const size, void * __restrict const data)
			 * Store @size bytes of the provided @data into the currently
			 * bound GL_ARRAY_BUFFER, starting at offset 0.
			 * 
			 * WARNING
			 * This does *NOT* check if any GL_ARRAY_BUFFER is bound, or if
			 * the currently bound buffer is from this buffer set !
			 * 
			 * Params :
			 * @param size The number of bytes to copy from @data
			 * @param data The data to store into the currently bound GPU
			 *             buffer
			 */
			void store(uint32_t const size, void * __restrict const data);
			
			
			
		};


	}
}
