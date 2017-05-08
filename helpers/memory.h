#ifndef MYY_HELPERS_MEMORY_H
#define MYY_HELPERS_MEMORY_H 1

#include <stdint.h>

void * allocate_durable_memory
(unsigned int const bytes_amount);

void * reallocate_durable_memory
(void * __restrict const addr,
 unsigned int const bytes_amount);

void free_durable_memory
(void * __restrict const addr);

void * allocate_temporary_memory
(unsigned int const bytes_amount);

void * reallocate_temporary_memory
(void * __restrict const addr,
 unsigned int const bytes_amount);

void free_temporary_memory
(void * __restrict const addr);

void * clean_memory_space
(void * __restrict const space,
 unsigned int const size);

void * recopy_inside_memory_space
(void * __restrict const to,
 void * __restrict const from,
 unsigned int const size);

#endif
