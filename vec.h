 /* VEC 1.0.0 -- A C library all about generic vectors.
 *
 * ABOUT:
 * 		The VEC library is meant to add generic and fast vectors to C (and is compatible with C++). A
 * 		"vec" in a sense is a fat pointer to anything. The VEC API not only provides ways of creating, 
 * 		destroying, and manipulating generic fat pointers, but also functionality common to the traditional C++ 
 * 		vector type.  
 *
 * 		The use of fat pointers helps prevent the problems associated with how arrays are downgraded to pointers 
 * 		when passed to functions. If you are unfamiliar with the concept of fat pointers, I suggest reading up 
 * 		on it because they are extremely useful. There are two major downsides to using fat pointers:
 * 			1. Fat pointers require the use of custom allocator functions.
 * 			2. Fat pointers traditionally need to be reassigned by a function's return value when operated on.
 *
 * 		Problem 1 is solved easily by providing these custom allocator functions (such as 'vec_new(...)' and
 * 		'vec_free(...)'). Problem 2 is a little more prestigious, and requires the understanding of how vec's work
 * 		internally in the API. Whenever a vec is added to, the capacity of it may not be big enough, so it may
 * 		need to reallocate. In such a case, the pointer to the vec will have to be reassigned appropriately.
 * 		Note that this is only the case in functions that operate on the capacity (not the length) of the
 * 		vec. The easiest solution for the end-user is the use of macro-functions, since they operate under the
 * 		enclosing scope, rather than a completely seperate scope. It may not be immediately apparent as to why
 * 		macro-functions are so useful in this case, but the examples shown on github will immediately explain
 * 		its practicality. If, for whatever reason, the respective function cannot carry out its task on the
 * 		vec because it is out of memory, it will raise a segment violation.
 *
 * 		Fat pointer diagram:
 *			+--------+-------------------------------------------+
 * 			| Header | First_Item    Second_Item    Third_Item...|
 * 			+--------+-------------------------------------------+
 *				 	 |
 *				 	 `-> Pointer returned to the user.
 *
 * 		Information stored in the header:
 * 			- capacity
 * 			- length
 * 			- sizeof the stored item
 *
 * USAGE:
 * 		This library is self contained in one single header file and can be used either in header only
 * 		mode or in implementation mode. The header only mode is used by default when included and allows
 * 		including this header in other headers and does not contain the actual implementation.
 *
 * 		The implementation mode requires to define the preprocessor macro VEC_IMPLEMENTATION in *one*
 * 		.c/.cpp file before #including this file, e.g.:
 *
 * 			#define VEC_IMPLEMENTATION
 * 			#include "vec.h"
 *
 * 		Also optionally define the symbols listed in the section "OPTIONAL DEFINES" below *only* in
 * 		implementation mode if you want to use additional functionality or need more control over the
 * 		library. 
 *
 * OPTIONAL DEFINES:
 * 		VEC_PRIVATE
 * 			If defined declares all functions as static, so they can only be accessed inside the file
 * 			that contains the implementation.
 *
 * 		VEC_MAX_PREALLOC (<max prealloc size>)
 * 			By default, the maximum preallocation size of a vec is one megabyte. 
 * 		
 * 		VEC_MALLOC <func name>
 * 			By default, uses 'malloc' from the stdlib. 
 *
 * 		VEC_REALLOC <func name>
 * 			By default, uses 'realloc' from the stdlib.
 *
 * 		VEC_FREE <func name>
 * 			By default, uses 'free' from the stdlib. 
 *
 * FAQ:
 * 		1. Is there a difference between 'vec_newlen(...)/vec_len(...)' and 'vec_newsize(...)/vec_size(...)'?
 * 			- No. Some people prefer "len" over "size" and vice versa.
 * 		2. What does 'VEC_MAX_PREALLOC' do?
 * 			- This sets the maximum (initially unused) additional space a vec can have in memory.
 * 		3. Wouldn't using a structure with a flexible array member be less complicated in the
 * 		   implementation?
 * 		    - The reason a structure could not be used to represent the vec's header and its contents was
 * 		      because vec's in this library are "generic", so pointers to void are used exclusively. 
 * 		      Unfortunately, you can't have a flexible array member be void (e.g.: 'void foo[];').
 * 		4. Why isn't 'VEC_CALLOC/calloc' used/defined?
 * 			- Because there would always have to be two seperate allocations (one for the header, and the
 * 			  other for the vec's contents) if it were used, which is a no-go.
 * 		5. What standard was this library written under?
 * 			- C99.
 * 		6. Why do the macro functions in the VEC API use 'do { ... } while (0)' constructs?
 * 			- It ensures the macro always behaves the same, regardless of how semicolons and curly
 * 			  brackets are used in invoking the code.
 * 		7. Why does the API raise 'SIGSEGV' when out of memory?
 * 			- First of all, there is no way for the user to detect whether or not the vec could be
 * 			  properly allocated based on the respective function's return value. From the user's
 * 			  perspective, a vec can be equal to NULL and still be valid. One may argue that the
 * 			  functions which don't create new vec's, but rather add to a vec already created, should
 * 			  just do nothing to the vec and return silently in such a scenario. However, this would give 
 * 			  way to a memory condition (like a race condition) in which more than likely the outcome of 
 * 			  the user's program would be unintentionally different depending on the scenario.
 */
#ifndef VEC_H_
#define VEC_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>


/*
 * ========================================================
 *
 * 						OPTIONAL DEFINES
 *
 * ========================================================
 */
#ifndef VEC_API
	#ifdef VEC_PRIVATE
		#define VEC_API static
	#else
		#define VEC_API extern
	#endif
#endif

#ifndef VEC_MAX_PREALLOC
	#define VEC_MAX_PREALLOC (1024*1024)
#endif

#ifndef VEC_MALLOC
	#define VEC_MALLOC malloc
#endif

#ifndef VEC_REALLOC
	#define VEC_REALLOC realloc
#endif

#ifndef VEC_FREE
	#define VEC_FREE free
#endif


/*
 * ========================================================
 *
 * 						VEC API
 *
 * ========================================================
 */ 
VEC_API		void*	vec_new(size_t size);
VEC_API 	void*	vec_newlen(size_t size, size_t nitems);
VEC_API 	void*	vec_newsize(size_t size, size_t nitems);
VEC_API 	void*	vec_dup(void *vec);
VEC_API		void 	vec_free(void *vec);
VEC_API		size_t	vec_cap(void *vec);
VEC_API 	size_t	vec_len(void *vec);
VEC_API		size_t	vec_size(void *vec);
VEC_API		bool	vec_isempty(void *vec);
VEC_API 	void	vec_clear(void *vec);
VEC_API 	void	vec_pop(void *vec);
VEC_API		void	vec_remove(void *vec, size_t index);

#define vec_push(vec, item)																						\
	do {																										\
		if (_vec_hdr_len(vec) == _vec_hdr_cap(vec)) {															\
			size_t new_cap, new_total_size;																		\
			if(_vec_hdr_cap(vec) <= VEC_MAX_PREALLOC) {															\
				new_cap = _vec_hdr_cap(vec) * 2;																\
				new_total_size = sizeof(size_t) * 3 + (new_cap) * _vec_hdr_item_size(vec);						\
			} else {																							\
				new_cap = _vec_hdr_cap(vec) + 1;																\
				new_total_size = sizeof(size_t) * 3 + (new_cap) * _vec_hdr_item_size(vec);						\
			}																									\
																												\
			void *temp = VEC_REALLOC(_vec_hdr_addr(vec), new_total_size);										\
			if (temp == NULL) {																					\
				raise(SIGSEGV);																					\
			}																									\
			vec = _vec_cast(vec) ((char *) temp + sizeof(size_t) * 3);											\
																												\
			_vec_hdr_cap(vec) = new_cap;																		\
		}																										\
																												\
		++_vec_hdr_len(vec);																					\
		vec[_vec_hdr_len(vec) - 1] = item;																		\
	} while(0)

#define vec_insert(vec, item, index)																			\
	do {																										\
		if ((size_t) index == index && index >= 0 && index <= _vec_hdr_len(vec)) {								\
			if (_vec_hdr_len(vec) == _vec_hdr_cap(vec)) {														\
				size_t new_cap, new_total_size;																	\
				if(_vec_hdr_cap(vec) <= VEC_MAX_PREALLOC) {														\
					new_cap = _vec_hdr_cap(vec) * 2;															\
					new_total_size = sizeof(size_t) * 3 + (new_cap) * _vec_hdr_item_size(vec);					\
				} else {																						\
					new_cap = _vec_hdr_cap(vec) + 1;															\
					new_total_size = sizeof(size_t) * 3 + (new_cap) * _vec_hdr_item_size(vec);					\
				}																								\
																												\
				void *temp = VEC_REALLOC(_vec_hdr_addr(vec), new_total_size);									\
				if (temp == NULL) {																				\
					raise(SIGSEGV);																				\
				}																								\
				vec = _vec_cast(vec) ((char *) temp + sizeof(size_t) * 3);										\
																												\
				_vec_hdr_cap(vec) = new_cap;																	\
			}																									\
																												\
			++_vec_hdr_len(vec);																				\
			memmove(																							\
				(char *) vec + (index + 1) * _vec_hdr_item_size(vec),											\
				(char *) vec + index * _vec_hdr_item_size(vec),													\
				_vec_hdr_item_size(vec) * ((_vec_hdr_len(vec) - 1) - index)										\
			);																									\
			vec[index] = item;																					\
		}																										\
	} while(0)

#define vec_swap(vec, vec_2)																					\
	do {																										\
		void *temp = vec;																						\
		vec = vec_2;																							\
		vec_2 = _vec_cast(vec) temp;																			\
	} while(0)																

#define vec_reserve(vec, size)																					\
	do {																										\
		if ((size_t) size == size && size > _vec_hdr_cap(vec)) {												\
			void *temp = VEC_REALLOC(_vec_hdr_addr(vec), sizeof(size_t) * 3 + size * _vec_hdr_item_size(vec));	\
			if (temp == NULL) {																					\
				raise(SIGSEGV);																					\
			}																									\
			vec = _vec_cast(vec) ((char *) temp + sizeof(size_t) * 3);											\
																												\
			_vec_hdr_cap(vec) = size;																			\
		}																										\
	} while(0)

#define vec_shrink(vec)																							\
	do {																										\
		if (_vec_hdr_cap(vec) != _vec_hdr_len(vec)) {															\
			size_t new_cap = _vec_hdr_len(vec) > 0 ? _vec_hdr_len(vec) : 1;										\
			void *temp = VEC_REALLOC(																			\
				_vec_hdr_addr(vec), sizeof(size_t) * 3 + new_cap * _vec_hdr_item_size(vec)						\
			);																									\
			if (temp == NULL) {																					\
				raise(SIGSEGV);																					\
			}																									\
			vec = _vec_cast(vec) ((char *) temp + sizeof(size_t) * 3);											\
																												\
			_vec_hdr_cap(vec) = new_cap;																		\
		}																										\
	} while(0)


/*
 * ========================================================
 *
 * 						LOW-LEVEL FUNCTIONALITY
 *
 * ========================================================
 */
// General Accessors/Helpers for the VEC API.
#define _vec_hdr_addr(vec) ((char *) vec - sizeof(size_t) * 3)
#define _vec_hdr_cap(vec) (* (size_t *) (_vec_hdr_addr(vec)))
#define _vec_hdr_len(vec) (* (size_t *) (_vec_hdr_addr(vec) + sizeof(size_t)))
#define _vec_hdr_item_size(vec) (* (size_t *) (_vec_hdr_addr(vec) + sizeof(size_t) * 2))
#define _vec_total_size(vec) (sizeof(size_t) * 3 + _vec_hdr_cap(vec) * _vec_hdr_item_size(vec))

// Helper macro-function exclusively for the macro-functions in the VEC API.
#ifdef __cplusplus
	#define _vec_cast(vec) (decltype(vec))
#else
	#define _vec_cast(vec) (void *)
#endif


#ifdef __cplusplus
}
#endif


/*
 * ========================================================
 *
 * 						IMPLEMENTATION
 *
 * ========================================================
 */
#ifdef VEC_IMPLEMENTATION


VEC_API inline void* vec_new(size_t size) {
	return vec_newlen(size, 1);
}


VEC_API inline void* vec_newlen(size_t size, size_t nitems) {
	void *vec = VEC_MALLOC(sizeof(size_t) * 3 + size * nitems);
	if (vec == NULL) {
		raise(SIGSEGV);
	}
	vec = (char *) vec + sizeof(size_t) * 3;

	_vec_hdr_cap(vec) = nitems;
	_vec_hdr_len(vec) = 0;
	_vec_hdr_item_size(vec) = size;
	memset(vec, 0, size * nitems);

	return vec;
}


VEC_API inline void* vec_newsize(size_t size, size_t nitems) {
	return vec_newlen(size, nitems);
}


VEC_API inline void* vec_dup(void *vec) {
	void *dup = VEC_MALLOC(sizeof(size_t) * 3 + _vec_hdr_cap(vec) * _vec_hdr_item_size(vec));
	if (dup == NULL) {
		raise(SIGSEGV);
	}
	dup = (char *) dup + sizeof(size_t) * 3;

	memcpy(_vec_hdr_addr(dup), _vec_hdr_addr(vec), _vec_total_size(vec));

	return dup;
}


VEC_API inline void vec_free(void *vec) {
	VEC_FREE(_vec_hdr_addr(vec));
}


VEC_API inline size_t vec_cap(void *vec) {
	return _vec_hdr_cap(vec);
}


VEC_API inline size_t vec_len(void *vec) {
	return _vec_hdr_len(vec);
}


VEC_API inline size_t vec_size(void *vec) {
	return _vec_hdr_len(vec);
}


VEC_API inline bool vec_isempty(void *vec) {
	return _vec_hdr_len(vec) == 0 ? 1 : 0;
}


VEC_API inline void vec_clear(void *vec) {
	_vec_hdr_len(vec) = 0;
}


VEC_API inline void vec_pop(void *vec) {
	if (_vec_hdr_len(vec) > 0) {
		--_vec_hdr_len(vec);
	}
}


VEC_API inline void vec_remove(void *vec, size_t index) {
	if (_vec_hdr_len(vec) > 0 && index < _vec_hdr_len(vec)) {
		memmove(
			(char *) vec + index * _vec_hdr_item_size(vec),
			(char *) vec + (index + 1) * _vec_hdr_item_size(vec),
			_vec_hdr_item_size(vec) * (_vec_hdr_len(vec) - (index + 1))
		);

		--_vec_hdr_len(vec);
	}
}


#endif


#endif