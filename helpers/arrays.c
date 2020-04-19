#include <myy/helpers/arrays.h>
#include <myy/helpers/memory.h>

#include <stdint.h>
#include <stddef.h> // NULL !!?

void split_array_init
(SplitArray * __restrict const split_array,
 unsigned int const elements, unsigned int const element_size)
{
	unsigned int const bytesize = element_size * elements;
	uint8_t * __restrict const array_address =
		(uint8_t *) allocate_temporary_memory(bytesize);
	clean_memory_space(array_address, bytesize);

	split_array->data = array_address;
	split_array->elements.total = elements;
	split_array->elements.count[split_array_left] = 0;
	split_array->elements.count[split_array_right] = 0;
}

uint8_t dyn_array_generic_ensure_space_for_more_u16
(DynArray_u16_t * __restrict const dyn_array,
 uint16_t const at_least_n_more, uint_fast32_t const element_size)
{
	void * reallocated_address = dyn_array->data;
	unsigned int at_least = dyn_array->count + at_least_n_more;
	if (at_least > dyn_array->max) {
		reallocated_address = reallocate_durable_memory(
			dyn_array->data, at_least * element_size
		);
		if (reallocated_address != NULL) {
			dyn_array->data = reallocated_address;
			dyn_array->max = at_least;
		}
	}
	return reallocated_address != NULL;
}

void dyn_array_data_pointers_init
(DynPointers_u16_t * __restrict const dyn_array, uint16_t const max)
{
	dyn_array->data  = 
		(void **) allocate_durable_memory(max * sizeof(void *));
	dyn_array->count = 0;
	dyn_array->max   = max;
}

uint8_t dyn_array_data_pointers_grow_u16
(DynPointers_u16_t * __restrict const dyn_array, uint16_t at_least)
{
	void * reallocated_address = NULL;
	if (at_least > dyn_array->max) {
		reallocated_address = reallocate_durable_memory(
			dyn_array->data, at_least * sizeof(void *)
		);
	}
	return reallocated_address != NULL;
}

uint8_t dyn_array_data_pointers_append_u16
(DynPointers_u16_t * __restrict const dyn_array, void * data)
{
	uint8_t added = 0;

	uint8_t enough_space = dyn_array_generic_ensure_space_for_more_u16(
		(DynArray_u16_t *) dyn_array, 1, sizeof(void *)
	);

	if (enough_space) {
		dyn_array->data[dyn_array->count] = data;
		dyn_array->count++;
		added = 1;
	}
	return added;
}
