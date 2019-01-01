#ifndef MYY_VECTOR_H
#define MYY_VECTOR_H 1

#include <myy/helpers/c_types.h>

#define ALIGN_ON_POW2(x, p) ((x)+(p-1) & ~(p-1))

struct myy_vector {
	uintptr_t begin;
	uintptr_t tail;
	uintptr_t end;
};

typedef struct myy_vector myy_vector_t;

static inline bool myy_vector_can_add(
	struct myy_vector const * __restrict const vector,
	size_t const n_octets)
{
	return ((vector->tail + n_octets) < vector->end);
}

static inline size_t myy_vector_size(
	struct myy_vector const * __restrict const vector)
{
	return (size_t) (vector->end - vector->begin);
}

static inline size_t myy_vector_last_offset(
	struct myy_vector const * __restrict const vector)
{
	return vector->tail - vector->begin;
}

static inline bool myy_vector_expand_to_store_at_least(
	struct myy_vector * const vector,
	size_t const n_octets)
{
	size_t const vector_size     = myy_vector_size(vector);
	size_t const new_vector_size =
		ALIGN_ON_POW2((vector_size + n_octets), 4096);
	size_t const vector_last_offset =
		myy_vector_last_offset(vector);
	uintptr_t new_begin = (uintptr_t) realloc(
		(uint8_t * __restrict) vector->begin,
		new_vector_size);

	bool success = (new_begin != 0);

	if (success) {
		vector->begin = new_begin;
		vector->tail  = new_begin + vector_last_offset;
		vector->end   = new_begin + new_vector_size;
	}

	return success;
}

bool myy_vector_add(
	struct myy_vector * const vector,
	size_t const n_octets,
	uint8_t const * __restrict const source);


struct myy_vector myy_vector_init(
	size_t const n_octets);

static inline void myy_vector_free_content(
	struct myy_vector const vector)
{
	free((uint8_t * __restrict) vector.begin);
}

static inline bool myy_vector_is_valid(
	struct myy_vector const * __restrict const vector)
{
	return (((uint8_t const * __restrict) vector->begin) != NULL);
}

static inline uint8_t * myy_vector_data(
	struct myy_vector const * __restrict const vector)
{
	return (uint8_t * __restrict) (vector->begin);
}

static inline void myy_vector_move_tail_back(
	struct myy_vector * __restrict const vector,
	size_t const n_octets)
{
	vector->tail -= n_octets;
}

static inline void myy_vector_inspect(
	struct myy_vector * __restrict const vector)
{
	printf(
		"Begin : 0x%016lx\n"
		"Tail  : 0x%016lx\n"
		"End   : 0x%016lx\n",
		vector->begin, vector->tail, vector->end);
}

/*static inline uint16_t * myy_vector_data_uint16(
	struct myy_vector const * __restrict const vector)
{
	return (uint16_t const * __restrict) myy_vector_data(vector);
}

static inline struct myy_vector_init_uint16(
	size_t const n_elements)
{
	myy_vector_init(n_elements * sizeof(uint16_t));
}

static inline bool myy_vector_add_uint16(
	struct myy_vector * const vector,
	size_t const n_elements,
	uint16_t const * __restrict const source)
{
	return myy_vector_add(
		vector,
		n_elements * sizeof(uint16_t),
		(uint8_t const * __restrict) source);
}

static inline size_t myy_vector_stored_uint16(
	struct myy_vector * const vector)
{
	return myy_vector_last_offset(vector) / sizeof(uint16_t);
}*/

#define myy_vector_for_each(vector, T, name, ...) {\
	T const * __restrict _cursor =         \
		(T * __restrict) vector->begin;   \
	T const * __restrict const _end =            \
		(T * __restrict) vector->tail;     \
	while(_cursor < _end) {\
		T const name = *_cursor++; \
		__VA_ARGS__\
	}\
}\

#define myy_vector_ptr_at(vector, T, index) ((T *) vector+index)

#define myy_vector_at(vector, T, index) *(myy_vector_ptr_at(vector, T, index))

#define myy_vector_count(vector, T) (myy_vector_last_offset(vector) / sizeof(T))

#define myy_vector_for_each_ptr(vector, T, name, ...) {\
	T * __restrict _cursor =         \
		(T * __restrict) vector->begin;   \
	T const * __restrict const _end =            \
		(T * __restrict) vector->tail;     \
	while(_cursor < _end) {\
		T * __restrict const name = _cursor; \
		_cursor++; \
		__VA_ARGS__\
	}\
}\

#endif
