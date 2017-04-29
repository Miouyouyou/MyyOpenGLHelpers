#include <myy/helpers/opengl/buffers>

namespace Myy::GL {
	void TripleSimpleBuffers::initialize
	(uint32_t original_size)
	{
		
		glGenBuffers(3, this->id);
		glBindBuffer(GL_ARRAY_BUFFER, this->id[0]);
		glBufferData(GL_ARRAY_BUFFER, original_size, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, this->id[1]);
		glBufferData(GL_ARRAY_BUFFER, original_size, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, this->id[2]);
		glBufferData(GL_ARRAY_BUFFER, original_size, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, this->id[0]);
		
		this->size.max[2] = this->size.max[1] = this->size.max[0] =
			original_size;
	}
	
	void TripleSimpleBuffers::bind_next_buffer
	{
		uint8_t current_id = this->current_index;
		uint8_t new_index = (current_id != 2) ? current_id + 1 : 0;
		
		glBindBuffer(GL_ARRAY_BUFFER, this->id[new_index]);
		
		this->current_index = new_index;
	}
	
	void TripleSimpleBuffers::store
	(uint32_t const size, void * __restrict const data)
	{
		uint8_t current_index = current_index = this->current_index;
		if (size < this->size.max[current_index])
			glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
		else {
			uint32_t new_size = size * 2;
			glBufferData(GL_ARRAY_BUFFER, new_size, data, GL_DYNAMIC_DRAW);
			this->size.max[current_index] = new_size;
		}
	}
}
