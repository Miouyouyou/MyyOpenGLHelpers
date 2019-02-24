#ifndef MYY_VECTOR_H
#define MYY_VECTOR_H 1

#include <myy/helpers/c_types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

static inline bool myy_vector_can_store(
	struct myy_vector const * __restrict const vector,
	size_t const total_octets)
{
	return ((vector->begin + total_octets) < vector->end);
}

static inline size_t myy_vector_allocated_total(
	struct myy_vector const * __restrict const vector)
{
	return (size_t) (vector->end - vector->begin);
}

static inline size_t myy_vector_allocated_used(
	struct myy_vector const * __restrict const vector)
{
	return (size_t) (vector->tail - vector->begin);
}

static inline void myy_vector_reset(
	struct myy_vector * __restrict const vector)
{
	vector->tail = vector->begin;
}

static inline bool myy_vector_expand_to_store_at_least(
	struct myy_vector * const vector,
	size_t const n_octets)
{
	size_t const vector_size     =
		myy_vector_allocated_total(vector);
	size_t const new_vector_size =
		ALIGN_ON_POW2((vector_size + n_octets), 4096);
	size_t const vector_last_offset =
		myy_vector_allocated_used(vector);
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

static inline void myy_vector_forget_last(
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

/**
 * Move the all the content, starting from "index", to the new
 * index.
 * 
 * Mainly used to insert some content in the middle of the
 * vector.
 * 
 * @param vector
 * The vector to shift content in
 * 
 * @param from
 * The index where we should start shifting
 * 
 * @param to    
 * The new index where the content will be moved.
 * 
 * @return
 * true if the content was moved.
 * false otherwise.
 */


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

#define myy_vector_template(suffix, T)                                             \
                                                                                   \
	struct myy_vector_##suffix##_st {                                              \
		uintptr_t begin;                                                           \
		uintptr_t tail;                                                            \
		uintptr_t end;                                                             \
	};                                                                             \
                                                                                   \
	typedef struct myy_vector_##suffix##_st myy_vector_##suffix;                   \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline bool myy_vector_##suffix##_can_add(                              \
		myy_vector_##suffix const * __restrict const vector,                       \
		size_t const n_elements)                                                   \
	{                                                                              \
		return myy_vector_can_add(                                                 \
			(struct myy_vector const *) vector,                                    \
			n_elements * sizeof(T));                                               \
	}                                                                              \
                                                                                   \
    __attribute__((unused))                                                        \
	static inline bool myy_vector_##suffix##_can_store(                            \
		myy_vector_##suffix const * __restrict const vector,                       \
		size_t const total_elements)                                               \
	{                                                                              \
		return myy_vector_can_store(                                               \
			(struct myy_vector const *) vector,                                    \
			total_elements * sizeof(T));                                           \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline T * myy_vector_##suffix##_data(                                  \
		myy_vector_##suffix  * __restrict const vector)                            \
	{                                                                              \
		return (T *) (myy_vector_data((struct myy_vector *) vector));              \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline size_t myy_vector_##suffix##_allocated_total(                    \
		myy_vector_##suffix  const * __restrict const vector)                      \
	{                                                                              \
		return (size_t) (                                                          \
			myy_vector_allocated_total((struct myy_vector const *) vector)         \
		);                                                                         \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline size_t myy_vector_##suffix##_allocated_used(                     \
		myy_vector_##suffix  const * __restrict const vector)                      \
	{                                                                              \
		return (size_t) (                                                          \
			myy_vector_allocated_used((struct myy_vector const *) vector)          \
		);                                                                         \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline size_t myy_vector_##suffix##_length(                             \
		myy_vector_##suffix  const * __restrict const vector)                      \
	{                                                                              \
		return                                                                     \
			myy_vector_allocated_used((struct myy_vector *) vector)                \
			/ sizeof(T);                                                           \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline bool myy_vector_##suffix##_expand_to_store_at_least(             \
		myy_vector_##suffix  * __restrict const vector,                            \
		size_t const n_elements)                                                   \
	{                                                                              \
		return myy_vector_expand_to_store_at_least(                                \
			((struct myy_vector *) vector),                                        \
			n_elements * sizeof(T));                                               \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline bool myy_vector_##suffix##_add(                                  \
		myy_vector_##suffix  * __restrict const vector,                            \
		size_t const n_elements,                                                   \
		T const * __restrict const source)                                         \
	{                                                                              \
		myy_vector_add(                                                            \
			(struct myy_vector *) vector,                                          \
			n_elements * sizeof(T),                                                \
			(uint8_t const *) source);                                             \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline myy_vector_##suffix  myy_vector_##suffix##_init(                 \
		size_t n_elements)                                                         \
	{                                                                              \
		myy_vector_##suffix vector;                                                \
                                                                                   \
		size_t allocated_size = ALIGN_ON_POW2(n_elements * sizeof(T), 4096);       \
		uintptr_t const begin =                                                    \
			(uintptr_t) (aligned_alloc(4096, allocated_size));                     \
		memset((void *) begin, 0, allocated_size);                                 \
		vector.begin = begin;                                                      \
		vector.tail  = begin;                                                      \
		vector.end   = begin + allocated_size;                                     \
                                                                                   \
		return vector;                                                             \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline void myy_vector_##suffix##_free_content(                         \
		myy_vector_##suffix  const vector)                                         \
	{                                                                              \
		free((T*) (vector.begin));                                                 \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline bool myy_vector_##suffix##_is_valid(                             \
		myy_vector_##suffix  const * __restrict const vector)                      \
	{                                                                              \
		return myy_vector_is_valid((struct myy_vector const *) vector);            \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline void myy_vector_##suffix##_forget_last(                          \
		myy_vector_##suffix  * __restrict const vector,                            \
		size_t const n_elements)                                                   \
	{                                                                              \
		myy_vector_forget_last(                                                    \
			(struct myy_vector *) vector,                                          \
			(n_elements * sizeof(T)));                                             \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline void myy_vector_##suffix##_inspect(                              \
		myy_vector_##suffix  * __restrict const vector)                            \
	{                                                                              \
		printf(                                                                    \
			"Begin            : 0x%016lx\n"                                        \
			"Tail             : 0x%016lx\n"                                        \
			"End              : 0x%016lx\n"                                        \
			"N Elements       : %zu\n"                                             \
			"Allocated memory : %zu\n"                                             \
			"Used memory      : %zu\n",                                            \
			vector->begin, vector->tail, vector->end,                              \
			myy_vector_##suffix##_length(vector),                                  \
			myy_vector_##suffix##_allocated_total(vector),                         \
			myy_vector_##suffix##_allocated_used(vector));                         \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline T myy_vector_##suffix##_at(                                      \
		myy_vector_##suffix  * __restrict const vector,                            \
		size_t const index)                                                        \
	{                                                                              \
		return myy_vector_##suffix##_data(vector)[index];                          \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline T * myy_vector_##suffix##_ptr_at(                                \
		myy_vector_##suffix  * __restrict const vector,                            \
		size_t const index)                                                        \
	{                                                                              \
		return (myy_vector_##suffix##_data(vector)) + index;                       \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline size_t myy_vector_##suffix##_type_size()                         \
	{                                                                              \
		return sizeof(T);                                                          \
	}                                                                              \
	                                                                               \
	__attribute__((unused))                                                        \
	static inline void myy_vector_##suffix##_reset(                                \
		myy_vector_##suffix * __restrict const vector)                             \
	{                                                                              \
		return myy_vector_reset((struct myy_vector *) vector);                     \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline bool myy_vector_##suffix##_shift_from(                           \
		myy_vector_##suffix * __restrict const vector,                             \
		size_t const from,                                                         \
		size_t const to)                                                           \
	{                                                                              \
		/* TODO Can be simplified with "can_store" */                              \
		/* Hmm...                                            */                    \
		/* Can't use myy_vector_T_can_add that easily since  */                    \
		/* "size_t" is unsigned and "to - from" will give    */                    \
		/* absurd unsigned results if "to" is before "from", */                    \
		/* and fail myy_vector_T_can_add even though we      */                    \
		/* CLEARLY have enough space in that case.           */                    \
                                                                                   \
		bool ret = true;                                                           \
		if (to > from                                                              \
		    && !myy_vector_##suffix##_can_add(vector, to - from)                   \
			&& !myy_vector_##suffix##_expand_to_store_at_least(vector, to - from)) \
		{                                                                          \
			ret = false;                                                           \
			goto out;                                                              \
		}                                                                          \
                                                                                   \
		T * const from_address = myy_vector_##suffix##_ptr_at(vector, from);       \
		T * const to_address   = myy_vector_##suffix##_ptr_at(vector, to);         \
		size_t const n_bytes   =                                                   \
			(size_t)                                                               \
			( vector->tail - ((uintptr_t) from_address) );                         \
                                                                                   \
		memmove(to_address, from_address, n_bytes);                                \
                                                                                   \
		/* Readjust the end of the vector */                                       \
		vector->tail = ((uintptr_t) to_address) + n_bytes;                         \
                                                                                   \
	out:                                                                           \
		return ret;                                                                \
	}                                                                              \
                                                                                   \
	__attribute__((unused))                                                        \
	static inline bool myy_vector_##suffix##_write_at(                             \
		myy_vector_##suffix * __restrict const vector,                             \
		size_t const from,                                                         \
		T const * __restrict const new_elements,                                   \
		size_t const n_new_elements)                                               \
	{                                                                              \
		size_t const total = from + n_new_elements;                                \
		bool can_write =                                                           \
			myy_vector_##suffix##_can_store(vector, total)                         \
			|| myy_vector_##suffix##_expand_to_store_at_least(vector, total);      \
                                                                                   \
		if (can_write)                                                             \
			memcpy(                                                                \
				myy_vector_##suffix##_ptr_at(vector, from),                        \
				new_elements,                                                      \
				n_new_elements * sizeof(T));                                       \
                                                                                   \
		return can_write;                                                          \
	}

#endif
