#include <stdlib.h>
#include <stdint.h>
#include <string.h> // memset

#include <myy/helpers/memory.h>

void * allocate_durable_memory
(unsigned int const bytes_amount)
{
	return malloc(bytes_amount);
}

void * reallocate_durable_memory
(void * __restrict const addr,
 unsigned int const bytes_amount)
{
	return realloc(addr, bytes_amount);
}

void free_durable_memory
(void * __restrict const addr)
{
	free(addr);
	return;
}


void * allocate_temporary_memory
(unsigned int const bytes_amount)
{
	return malloc(bytes_amount);
}

void * reallocate_temporary_memory
(void * __restrict const addr,
 unsigned int const bytes_amount)
{
	return realloc(addr, bytes_amount);
}

void free_temporary_memory
(void * __restrict const addr)
{
	free(addr);
	return;
}

void * clean_memory_space
(void * __restrict const space,
 unsigned int const size)
{
	return memset(space, 0, size);
}

void * recopy_inside_memory_space
(void * __restrict const to,
 void * __restrict const from,
 unsigned int const size)
{
	return memmove(to, from, size);
}
