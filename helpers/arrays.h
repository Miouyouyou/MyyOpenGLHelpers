#ifndef MYY_HELPERS_ARRAYS_H
#define MYY_HELPERS_ARRAYS_H 1

#include <stdint.h>

//---------- Data structures and types

enum split_array_section {
	split_array_left,
	split_array_right,
	n_split_array_sections
};

struct split_array_metadata {
	uint16_t count[n_split_array_sections], total;
};

struct split_array {
	uint8_t * __restrict data;
	struct split_array_metadata elements;
};
typedef struct split_array SplitArray;

struct dyn_array_data_pointers_u16 {
	uint16_t count; uint16_t max;
	void ** data;
};

struct dyn_array_generic_u16 {
	uint16_t count; uint16_t max;
	void * data;
};

typedef struct dyn_array_generic_u16 DynArray_u16_t;

typedef struct dyn_array_data_pointers_u16 DynPointers_u16_t;

//------- Procedures


void split_array_init
(SplitArray * __restrict const split_array,
 unsigned int const elements, unsigned int const element_size);

inline static unsigned int split_array_get_count
(SplitArray * __restrict const split_array,
 enum split_array_section section)
{
	return split_array->elements.count[section];
}

inline static uint8_t dyn_array_generic_is_full_u16
(DynArray_u16_t const * __restrict const dyn_array)
{
	return dyn_array->count >= dyn_array->max;
}

uint8_t dyn_array_generic_ensure_space_for_more_u16
(DynArray_u16_t * __restrict const dyn_array,
 uint16_t at_least_n_more, uint_fast32_t size);

void dyn_array_data_pointers_init
(DynPointers_u16_t * __restrict const dyn_array, uint16_t const max);
uint8_t dyn_array_data_pointers_grow_u16
(DynPointers_u16_t * __restrict const dyn_array, uint16_t at_least);
uint8_t dyn_array_data_pointers_append_u16
(DynPointers_u16_t * __restrict const dyn_array, void * data);

#endif
