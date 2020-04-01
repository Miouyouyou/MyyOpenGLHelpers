#include <cstdint>


namespace myy {

	static inline void * memory_aligned_alloc(
		uint64_t const pow_2_align,
		uint64_t const n_bytes)
	{
		return aligned_alloc(pow_2_align, n_bytes);
	}

	static inline void memory_aligned_free(
		void * __restrict const address)
	{
		free(address);
	}

	static inline void memory_aligned_free(
		uint64_t address)
	{
		memory_aligned_free((void *) address);
	}

}
