#ifndef MYY_MEMORY_H
#define MYY_MEMORY_H 1

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus

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

#endif

#endif
