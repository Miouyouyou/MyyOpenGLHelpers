#pragma once

#include <cstdint>
#include <iostream>
#include <functional>

#include "myy_memory.h"

#include <initializer_list>

namespace myy {

#define MIGHT_BE_UNUSED __attribute__((unused))
#ifndef ALIGN_ON_POW2
#define ALIGN_ON_POW2(x, pow2) ( ( (x) + ((pow2)-1) ) & ~((pow2) - 1) )
#endif

	struct dynarray8 {
		uint64_t begin;
		uint64_t tail;
		uint64_t end;
		uint64_t padding; // align on 32 bits.

		MIGHT_BE_UNUSED
		static inline bool is_valid(
			dynarray8 const * __restrict const arr)
		{
			return (
				arr != nullptr &&
				((arr->begin  != 0)
				 & (arr->tail != 0)
				 & (arr->end  != 0)
				 & (arr->begin <= arr->tail)
				 & (arr->tail <= arr->end)));
		}

		MIGHT_BE_UNUSED
		static dynarray8 create(uint64_t const n_octets)
		{
			dynarray8 arr;

			uint64_t const mem_size =
				ALIGN_ON_POW2(n_octets, 4096);
			uint64_t const begin =
				(uint64_t) (memory_aligned_alloc(4096, mem_size));
			if (begin)
				memset((void *) begin, 0, mem_size);

			arr.begin = begin;
			arr.tail  = begin;
			arr.end   = begin + mem_size;

			return arr;
		}

		MIGHT_BE_UNUSED
		static void free_content(
			dynarray8 * __restrict const arr)
		{
			memory_aligned_free(arr->begin);
			arr->begin = ((uint64_t) nullptr);
			arr->tail  = ((uint64_t) nullptr);
			arr->end   = ((uint64_t) nullptr);
		}

		MIGHT_BE_UNUSED
		static inline void init(
			dynarray8 * __restrict const arr,
			uint64_t n_octets)
		{
			if (is_valid(arr))
				free_content(arr);
			dynarray8 const values = create(n_octets);

			arr->begin = values.begin;
			arr->tail  = values.tail;
			arr->end   = values.end;
		}

		MIGHT_BE_UNUSED
		static inline bool can_add(
			dynarray8 const * __restrict const arr,
			uint64_t const n_octets)
		{
			return (arr->tail + n_octets) < arr->end;
		}

		MIGHT_BE_UNUSED
		static inline bool can_store(
			dynarray8 const * __restrict const arr,
			uint64_t const n_octets)
		{
			return (arr->begin + n_octets) < arr->end;
		}

		MIGHT_BE_UNUSED
		static inline uint64_t mem_total(
			dynarray8 const * __restrict const arr)
		{
			return (arr->end - arr->begin);
		}

		MIGHT_BE_UNUSED
		static inline uint64_t mem_used(
			dynarray8 const * __restrict const arr)
		{
			return (arr->tail - arr->begin);
		}

		MIGHT_BE_UNUSED
		static inline void reset(
			dynarray8 * __restrict const arr)
		{
			arr->tail = arr->begin;
		}

		MIGHT_BE_UNUSED
		static inline bool mem_total_add_at_least(
			dynarray8 * __restrict const arr,
			size_t const n_octets)
		{
			size_t const dynarray8_size     =
				dynarray8::mem_total(arr);
			size_t const new_dynarray8_size =
				ALIGN_ON_POW2((dynarray8_size + n_octets), 4096);
			size_t const dynarray8_last_offset =
				dynarray8::mem_used(arr);
			uint64_t new_begin = (uint64_t) realloc(
				(uint8_t * __restrict) arr->begin,
				new_dynarray8_size);

			bool success = (new_begin != 0);

			if (success) {
				arr->begin = new_begin;
				arr->tail  = new_begin + dynarray8_last_offset;
				arr->end   = new_begin + new_dynarray8_size;
			}

			return success;
		}

		MIGHT_BE_UNUSED
		static inline bool mem_prepare_to_add(
			dynarray8 * __restrict const arr,
			size_t const n_octets)
		{
			bool success = false;

			if (dynarray8::can_add(arr, n_octets)
			    || dynarray8::mem_total_add_at_least(arr, n_octets))
			{
				success = true;
			}

			return success;
		}

		MIGHT_BE_UNUSED
		static inline bool add_memcpy(
			dynarray8 * __restrict const arr,
			size_t const n_octets,
			uint8_t const * __restrict const source)
		{
			bool const can_add = dynarray8::can_add(arr, n_octets);
			bool added = false;

			if (can_add ||
				dynarray8::mem_total_add_at_least(arr, n_octets))
			{
				memcpy(
					(uint8_t * __restrict) arr->tail,
					source,
					n_octets);
				arr->tail += n_octets;
				added = true;
			}

			return added;
		}

		/* TODO
		 * -- Delete functions
		 * Delete at
		 * Shrink to
		 * -- Add functions
		 * Insert at
		 */
		MIGHT_BE_UNUSED
		static inline void remove_at(
			dynarray8 * __restrict const array,
			uint64_t const from,
			uint64_t const n_bytes)
		{
			/* This is a bit confusing.
			 * When you remove data at the provided
			 * location, you're basically moving after the
			 * provided location + n_bytes back to the
			 * provided location.
			 *
			 * We'll make a very simple check to ensure
			 * we're not reading past the allocated memory.
			 *
			 * Though, that check can actually make this
			 * function untrustful, since it might not
			 * remove anything at all...
                         */
			uint64_t const allocated =
				dynarray8::mem_used(array);

			uint64_t const asked_offset =
				from+n_bytes;

			uint64_t const from_offset =
				(from < allocated)
				? from
				: allocated;
			uint64_t const after_offset =
				(asked_offset < allocated)
				? asked_offset
				: allocated;

			uint64_t const moved_amount =
				allocated - after_offset;

			uint8_t * __restrict const array_begin =
				(uint8_t * __restrict const) (array->begin);

			memmove(
				array_begin+from_offset,
				array_begin+after_offset,
				moved_amount);
			array->tail = 
				(uint64_t)
				(array_begin+from_offset+moved_amount);
		}

		MIGHT_BE_UNUSED
		static inline void move_tail_back(
			dynarray8 * __restrict const array,
			uint64_t const n_bytes)
		{
			uint64_t const used_memory =
				dynarray8::mem_used(array);
			uint64_t const n_bytes_back =
				(n_bytes < used_memory)
				? n_bytes
				: used_memory;
			array->tail -= n_bytes_back;
		}

		MIGHT_BE_UNUSED
		static inline bool insert_at(
			dynarray8 * __restrict const array,
			uint64_t const at,
			uint8_t const * __restrict const src,
			uint64_t const n_bytes)
		{
			bool const prepared = dynarray8::mem_prepare_to_add(
				array, n_bytes);

			bool inserted = false;

			if (prepared) {
				/* Lot of parentheses to ensure that casts and pointer
				 * arithmetics are done in proper order.
				 */
				uint8_t * __restrict const insert_point =
					((uint8_t * __restrict) (array->begin))
					+ at;

				uint64_t const mem_used = array->tail;
				uint64_t const n_bytes_to_move =
					mem_used - at;

				/* Prepare the insertion by shifting all the data.
				 * Basically, we're moving everything from the insertion
				 * point 'n_bytes' away. */
				memmove(
					insert_point+n_bytes, /* dst */
					insert_point,         /* src */
					n_bytes_to_move);

				memcpy(
					insert_point,
					src,
					n_bytes);

				array->tail += n_bytes;
				inserted = true;
			}

			return inserted;
		}
	};

	template <typename T>
	struct dynarray {
		uint64_t begin;
		uint64_t tail;
		uint64_t end;
		uint64_t padding; // align on 32 bits.

		/* No constructor.
		 * 
		 * The dubious call of constructors and destructors at moments
		 * where I expect them the least is my main reason for not
		 * providing actual constructors for a lot C++ classes, unless
		 * the constructors do very basic things (set values to 0).
		 * Instead, I tend to use constructor functions... some people
		 * call these "factories". Factories have their own issues
		 * with destructors and RAII in C++.
		 * So there's no automatic destruction of allocated ressources
		 * in the destructor of ~dynarray.
		 * You have to call free_content() instead.
		 * You could of course modify ~dynarray to call free_content() if
		 * you want automatic destruction... in which case, you'll have
		 * to add an actual constructor that does NOT depend on
		 * create_with_size, since RAII will trigger the automatic
		 * destruction of the created "dynarray" after calling
		 * create_with_size. The reason being that the created object
		 * should only be valid in the current function scope.
		 * So something like :
		 * dynarray(size_t const size) { *this = create_with_size(size); }
		 * Will actually be equivalent to this, with RAII :
		 * dynarray(size_t const size) {
		 * 	auto arr = create_with_size(size);
		 * 	*this = arr;
		 * 	~arr();
		 * }
		 * If you free the resources during the destruction, then
		 * the three pointers stored in *this will point to unmapped areas !
		 */
		dynarray() :
			begin(0),
			tail(0),
			end(0) {}

		dynarray(
			uint64_t const obegin,
			uint64_t const otail,
			uint64_t const oend) :
			begin(obegin),
			tail(otail),
			end(oend)
		{}

		dynarray(dynarray & oarray) :
			begin(oarray.begin),
			tail(oarray.tail),
			end(oarray.end)
		{ }

		dynarray(dynarray const & oarray) :
			begin(oarray.begin),
			tail(oarray.tail),
			end(oarray.end)
		{ }

		dynarray(dynarray && oarray) :
			begin(oarray.begin),
			tail(oarray.tail),
			end(oarray.end)
		{ }

		dynarray(dynarray const && oarray) :
			begin(oarray.begin),
			tail(oarray.tail),
			end(oarray.end)
		{ }


		dynarray(std::initializer_list<T> list) :
			begin(0),
			tail(0),
			end(0)
		{
			this->init(list.size());
			this->add_memcpy(list.begin(), list.size());
		}

		dynarray<T>& operator=(dynarray<T> const & oarray) {
			this->begin = oarray.begin;
			this->tail  = oarray.tail;
			this->end   = oarray.end;
			return *this;
		}

		dynarray<T>& operator=(dynarray<T> & oarray) {
			this->begin = oarray.begin;
			this->tail  = oarray.tail;
			this->end   = oarray.end;
			return *this;
		}
		MIGHT_BE_UNUSED
		static inline dynarray<T> create_with_size(
			uint64_t const n_octets)
		{
			/* You can't cast structures...
			 * In some way, I understand why compilers don't
			 * authorize this... if the memberes don't match
			 * perfectly, it would be a real pain in the ass
			 * to deal with on the various architectures, for
			 * like 1% of potential use, which can be circumvented
			 * anyway.
			 * 
			 * So the "way around" is to just recreate an
			 * object with the same data.
			 */
			auto array = dynarray8::create(n_octets);

			return dynarray<T>(array.begin, array.tail, array.end);
		}

		inline void init() {
			dynarray8::init((dynarray8 * __restrict) this, 4096);
		}

		inline void init(uint64_t const n_elements) {
			uint64_t const size =
				ALIGN_ON_POW2((n_elements*sizeof(T)), 4096);
			dynarray8::init((dynarray8 * __restrict) this, size);
		}

		dynarray(
			T const * __restrict const values,
			size_t const n_elements) :
			begin(0),
			tail(0),
			end(0)
		{
			init(n_elements);
			if (is_valid())
				add_memcpy(values, n_elements);
		}

		inline void inspect() const {
			std::cerr << std::hex <<
				"Pointers\n" <<
				"--------\n" <<
				"\tBegin : " << this->begin << "\n" <<
				"\tTail  : " << this->tail << "\n" <<
				"\tEnd   : " << this->end << "\n" <<
				std::dec <<
				"In elements\n" <<
				"-----------\n" <<
				"\tStored : " << this->length() << "\n" <<
				"\tMax    : " << this->length_total() << "\n" <<
				"\n" <<
				"In bytes\n" <<
				"--------\n" <<
				"\tUsed      : " << this->mem_used() << " bytes\n" <<
				"\tAllocated : " << this->mem_total() << " bytes\n\n";
		}
		
		inline void inspect(
			char const * __restrict const name)
		{
			std::cerr << "[DYNARRAY : " << name << "]\n";
			inspect();
		}

		MIGHT_BE_UNUSED
		static inline dynarray<T> create(uint64_t const n_elements)
		{
			return create_with_size(sizeof(T) * n_elements);
		}

		MIGHT_BE_UNUSED
		static inline dynarray<T> create() {
			return create_with_size(4096 / sizeof(T));
		}

		inline bool is_valid() const {
			return dynarray8::is_valid(
				(dynarray8 const * __restrict) this);
		}

		~dynarray() {
		}

		inline void free_content() {
			dynarray8::free_content((dynarray8 * __restrict) this);
		}

		inline void free() {
			dynarray8::free_content((dynarray8 * __restrict) this);
		}

		inline bool can_add(uint64_t const n_elements) const
		{
			return dynarray8::can_add(
				(dynarray8 const * __restrict) this,
				n_elements * sizeof(T));
		}

		inline bool can_store(uint64_t const n_elements) const
		{
			return dynarray8::can_store(
				(dynarray8 const * __restrict) this,
				n_elements * sizeof(T));
		}

		inline uint64_t mem_total() const {
			return dynarray8::mem_total(
				(dynarray8 const * __restrict) this);
		}

		inline uint64_t mem_used() const {
			return dynarray8::mem_used(
				(dynarray8 const * __restrict) this);
		}

		inline void reset() {
			dynarray8::reset((dynarray8 * __restrict) this);
		}

		inline bool mem_total_add_at_least(uint64_t n_elements)
		{
			return dynarray8::mem_total_add_at_least(
				(dynarray8 * __restrict) this,
				n_elements * sizeof(T));
		}

		inline uint64_t length() const {
			return this->mem_used() / sizeof(T);
		}

		inline uint64_t length_total() const {
			return this->mem_total() / sizeof(T);
		}

		inline uint64_t last_idx() const {
			return this->length() - 1;
		}

		/* This add the element using memcpy.
		 *
		 * WARNING The copy constructors will NOT be called with this
		 * signature !
		 *
		 */
		inline bool add_memcpy(T const element) {
			return dynarray8::add_memcpy(
				(dynarray8 * __restrict) this,
				sizeof(T),
				(uint8_t const * __restrict) &element);
		}

		/* This add the elements using memcpy.
		 *
		 * WARNING The copy constructors will NOT be called with this
		 * signature !
		 *
		 */
		inline bool add_memcpy(
			uint64_t const n_elements,
			T const * __restrict const elements)
		{
			return dynarray8::add_memcpy(
				(dynarray8 * __restrict) this,
				sizeof(T) * n_elements,
				(uint8_t const * __restrict) elements);
		}

		/* This add the elements using memcpy.
		 *
		 * WARNING The copy constructors will NOT be called with this
		 * signature !
		 *
		 */
		inline bool add_memcpy(
			T const * __restrict const elements,
			uint64_t const n_elements)
		{
			return dynarray8::add_memcpy(
				(dynarray8 * __restrict) this,
				sizeof(T) * n_elements,
				(uint8_t const * __restrict) elements);
		}

		inline bool append_memcpy(T const element) {
			return add_memcpy(element);
		}
		inline bool append_memcpy(
			uint64_t const n_elements,
			T const * __restrict elements)
		{
			return add_memcpy(n_elements, elements);
		}
		inline bool append_memcpy(
			T const * __restrict elements,
			uint64_t const n_elements)
		{
			return add_memcpy(elements, n_elements);
		}

		inline T * array() const {
			return ((T *) this->begin);
		}
		inline T * data() const {
			return this->array();
		}

		inline T operator[](uint64_t index) const {
			return this->data()[index];
		}

		inline T at(uint64_t index) const {
			return this->data()[index];
		}

		inline T * at_ptr(uint64_t index) {
			return this->data()+index;
		}

		inline uint64_t offset_of(uint64_t index) const {
			return index * this->type_size();
		}

		inline T * last_ptr() {
			return this->at_ptr(this->last_idx());
		}
		inline T last() const {
			return this->at(this->last_idx());
		}

		inline T * tail_ptr() {
			return (T*) tail;
		}

		inline T const * tail_cptr() {
			return (T const *) tail;
		}

		inline constexpr size_t type_size() const {
			return sizeof(T);
		}

		inline void remove_at(uint64_t const index) {
			return dynarray8::remove_at(
				(dynarray8 * __restrict) this,
				this->offset_of(index),
				this->type_size());
		}

		inline bool remove_at_checked(uint64_t const index) {
			bool const should_remove =
				(index <= this->last_idx());
			if (should_remove) {
				this->remove_at(index);
			}
			return should_remove;
		}

		inline void remove_at(
			uint64_t const index,
			uint64_t const n_elements)
		{
			return dynarray8::remove_at(
				(dynarray8 * __restrict) this,
				this->offset_of(index),
				this->type_size()*n_elements);
		}

		inline bool remove_at_checked(
			uint64_t const index,
			uint64_t const n_elements)
		{
			bool const should_remove =
				((index + n_elements) <= last_idx());
			if (should_remove)
				this->remove_at(index, n_elements);
			return should_remove;
		}

		inline void remove_last() {
			dynarray8::move_tail_back(
				(dynarray8 * __restrict) this,
				this->type_size());
		}

		inline void move_tail_back() { remove_last(); }

		inline void remove_if(
			std::function<bool(T const *)> remove_condition)
		{
			uint64_t n_elements = this->length();
			T * __restrict const data = this->data();
			for (uint64_t i = 0; i < n_elements; i++)
			{
				T * __restrict const current_elt = data+i;
				if (remove_condition(current_elt)) {
					/* Remove at will actually move
					 * everything after index i at index i.
					 * So after remove_at(i), data+i will
					 * be different.
					 * 
					 * We compensate the loop i++ with --i so
					 * that the next iteration is done using
					 * the same index.
					 */
					this->remove_at(i);
					--n_elements;
					--i; 
				}
			}
			
		}

		inline void remove_last(uint64_t n_elements) {
			dynarray8::move_tail_back(
				(dynarray8 * __restrict) this,
				this->type_size() * n_elements);
		}
		inline void move_tail_back(uint64_t const n_elements) {
			remove_last(n_elements);
		}

		inline bool insert_at_memcpy(
			uint64_t const index,
			T const * __restrict const element)
		{
			return dynarray8::insert_at(this,
				(uint8_t const * __restrict) (this->offset_of(index)),
				(uint8_t const * __restrict) element,
				this->type_size());
		}

		inline bool insert_at_memcpy(
			uint64_t index,
			T const * __restrict const elements,
			uint64_t const n_elements)
		{
			return dynarray8::insert_at(
				(dynarray8 * __restrict) this,
				this->offset_of(index),
				(uint8_t const * __restrict) elements,
				this->type_size() * n_elements);
		}

		inline void for_each(
			std::function<void(T const * __restrict)> f) const
		{
			T const * __restrict cursor = this->data();
			T const * __restrict const tail =
				(T const * __restrict) (this->tail);
			while(cursor < tail) {
				f(cursor);
				cursor++;
			}
		}

		inline void for_each(
			std::function<void(T * __restrict)> f)
		{
			T * __restrict cursor = this->data();
			T const * __restrict const tail =
				(T const * __restrict) (this->tail);
			while (cursor < tail) {
				f(cursor);
				cursor++;
			}
		}
	};
};
