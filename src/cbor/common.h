/*
 * Copyright (c) 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_COMMON_H
#define LIBCBOR_COMMON_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "cbor/cbor_export.h"
#include "cbor/configuration.h"
#include "data.h"

#ifdef __cplusplus
extern "C" {

/**
 * C99 is not a subset of C++ -- 'restrict' qualifier is not a part of the
 * language. This is a workaround to keep it in C headers -- compilers allow
 * linking non-restrict signatures with restrict implementations.
 *
 * If you know a nicer way, please do let me know.
 */
#define CBOR_RESTRICT_POINTER

#else

// MSVC + C++ workaround
#define CBOR_RESTRICT_POINTER CBOR_RESTRICT_SPECIFIER

#endif

static const uint8_t cbor_major_version = CBOR_MAJOR_VERSION;
static const uint8_t cbor_minor_version = CBOR_MINOR_VERSION;
static const uint8_t cbor_patch_version = CBOR_PATCH_VERSION;

#define CBOR_VERSION               \
  _CBOR_TO_STR(CBOR_MAJOR_VERSION) \
  "." _CBOR_TO_STR(CBOR_MINOR_VERSION) "." _CBOR_TO_STR(CBOR_PATCH_VERSION)
#define CBOR_HEX_VERSION \
  ((CBOR_MAJOR_VERSION << 16) | (CBOR_MINOR_VERSION << 8) | CBOR_PATCH_VERSION)

/* http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
 */
#ifdef DEBUG
#include <stdio.h>
#define _cbor_debug_print(fmt, ...)                                     \
  do {                                                                  \
    if (DEBUG)                                                          \
      fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, \
              __VA_ARGS__);                                             \
  } while (0)
extern bool _cbor_enable_assert;
// Like `assert`, but can be dynamically disabled in tests to allow testing
// invalid behaviors.
#define CBOR_ASSERT(e) assert(!_cbor_enable_assert || (e))
#define _CBOR_TEST_DISABLE_ASSERT(block) \
  do {                                   \
    _cbor_enable_assert = false;         \
    block _cbor_enable_assert = true;    \
  } while (0)
#else
#define debug_print(fmt, ...) \
  do {                        \
  } while (0)
#define CBOR_ASSERT(e)
#define _CBOR_TEST_DISABLE_ASSERT(block) \
  do {                                   \
    block                                \
  } while (0)
#endif

#define _CBOR_TO_STR_(x) #x
#define _CBOR_TO_STR(x) _CBOR_TO_STR_(x) /* enables proper double expansion */

#ifdef CBOR_HAS_NODISCARD_ATTRIBUTE
#define CBOR_NODISCARD [[nodiscard]]
#else
#define CBOR_NODISCARD
#endif

#ifdef __GNUC__
#define _CBOR_UNUSED __attribute__((__unused__))
// Fall back to __attribute__((warn_unused_result)) if we don't have
// [[nodiscard]]
#ifndef CBOR_HAS_NODISCARD_ATTRIBUTE
#define _CBOR_NODISCARD __attribute__((warn_unused_result))
#endif
#elif defined(_MSC_VER)
#define _CBOR_UNUSED __pragma(warning(suppress : 4100 4101))
#define _CBOR_NODISCARD
#else
#define _CBOR_UNUSED
#define _CBOR_NODISCARD
#endif

#ifdef CBOR_HAS_BUILTIN_UNREACHABLE
#define _CBOR_UNREACHABLE __builtin_unreachable()
#else
#define _CBOR_UNREACHABLE
#endif

typedef void* (*_cbor_malloc_t)(size_t);
typedef void* (*_cbor_realloc_t)(void*, size_t);
typedef void (*_cbor_free_t)(void*);

CBOR_EXPORT extern _cbor_malloc_t _cbor_malloc;
CBOR_EXPORT extern _cbor_realloc_t _cbor_realloc;
CBOR_EXPORT extern _cbor_free_t _cbor_free;

// Macro to short-circuit builder functions when memory allocation fails
#define _CBOR_NOTNULL(cbor_item) \
  do {                           \
    if (cbor_item == NULL) {     \
      return NULL;               \
    }                            \
  } while (0)

// Macro to short-circuit builders when memory allocation of nested data fails
#define _CBOR_DEPENDENT_NOTNULL(cbor_item, pointer) \
  do {                                              \
    if (pointer == NULL) {                          \
      _cbor_free(cbor_item);                        \
      return NULL;                                  \
    }                                               \
  } while (0)

/** Sets the memory management routines to use.
 *
 * By default, libcbor will use the standard library `malloc`, `realloc`, and
 * `free`.
 *
 * \rst
 * .. warning:: This function modifies the global state and should therefore be
 *  used accordingly. Changing the memory handlers while allocated items exist
 *  will result in a ``free``/``malloc`` mismatch. This function is not thread
 *  safe with respect to both itself and all the other *libcbor* functions that
 *  work with the heap.
 *
 * .. note:: `realloc` implementation must correctly support `NULL` reallocation
 *  (see e.g. http://en.cppreference.com/w/c/memory/realloc)
 * \endrst
 *
 * @param custom_malloc malloc implementation
 * @param custom_realloc realloc implementation
 * @param custom_free free implementation
 */
CBOR_EXPORT void cbor_set_allocs(_cbor_malloc_t custom_malloc,
                                 _cbor_realloc_t custom_realloc,
                                 _cbor_free_t custom_free);

/*
 * ============================================================================
 * Type manipulation
 * ============================================================================
 */

/** Get the type of the item
 *
 * @param item
 * @return The type
 */
_CBOR_NODISCARD
CBOR_EXPORT cbor_type cbor_typeof(
    const cbor_item_t* item); /* Will be inlined iff link-time opt is enabled */

/* Standard CBOR Major item types */

/** Does the item have the appropriate major type?
 * @param item the item
 * @return Is the item an #CBOR_TYPE_UINT?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_isa_uint(const cbor_item_t* item);

/** Does the item have the appropriate major type?
 * @param item the item
 * @return Is the item a #CBOR_TYPE_NEGINT?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_isa_negint(const cbor_item_t* item);

/** Does the item have the appropriate major type?
 * @param item the item
 * @return Is the item a #CBOR_TYPE_BYTESTRING?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_isa_bytestring(const cbor_item_t* item);

/** Does the item have the appropriate major type?
 * @param item the item
 * @return Is the item a #CBOR_TYPE_STRING?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_isa_string(const cbor_item_t* item);

/** Does the item have the appropriate major type?
 * @param item the item
 * @return Is the item an #CBOR_TYPE_ARRAY?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_isa_array(const cbor_item_t* item);

/** Does the item have the appropriate major type?
 * @param item the item
 * @return Is the item a #CBOR_TYPE_MAP?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_isa_map(const cbor_item_t* item);

/** Does the item have the appropriate major type?
 * @param item the item
 * @return Is the item a #CBOR_TYPE_TAG?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_isa_tag(const cbor_item_t* item);

/** Does the item have the appropriate major type?
 * @param item the item
 * @return Is the item a #CBOR_TYPE_FLOAT_CTRL?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_isa_float_ctrl(const cbor_item_t* item);

/* Practical types with respect to their semantics (but not tag values) */

/** Is the item an integer, either positive or negative?
 * @param item the item
 * @return  Is the item an integer, either positive or negative?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_is_int(const cbor_item_t* item);

/** Is the item an a floating point number?
 * @param item the item
 * @return  Is the item a floating point number?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_is_float(const cbor_item_t* item);

/** Is the item an a boolean?
 * @param item the item
 * @return  Is the item a boolean?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_is_bool(const cbor_item_t* item);

/** Does this item represent `null`
 *
 * \rst
 * .. warning:: This is in no way related to the value of the pointer. Passing a
 *  null pointer will most likely result in a crash.
 * \endrst
 *
 * @param item the item
 * @return  Is the item (CBOR logical) null?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_is_null(const cbor_item_t* item);

/** Does this item represent `undefined`
 *
 * \rst
 * .. warning:: Care must be taken to distinguish nulls and undefined values in
 *  C.
 * \endrst
 *
 * @param item the item
 * @return Is the item (CBOR logical) undefined?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_is_undef(const cbor_item_t* item);

/*
 * ============================================================================
 * Memory management
 * ============================================================================
 */

/** Increases the item's reference count by one
 *
 * Constant complexity; items referring to this one or items being referred to
 * are not updated.
 *
 * This function can be used to extend reference counting to client code.
 *
 * @param item Reference to an item
 * @return The input \p item
 */
CBOR_EXPORT cbor_item_t* cbor_incref(cbor_item_t* item);

/** Decreases the item's reference count by one, deallocating the item if needed
 *
 * In case the item is deallocated, the reference count of all items this item
 * references will also be #cbor_decref 'ed recursively.
 *
 * @param item Reference to an item. Will be set to `NULL` if deallocated
 */
CBOR_EXPORT void cbor_decref(cbor_item_t** item);

/** Decreases the item's reference count by one, deallocating the item if needed
 *
 * Convenience wrapper for #cbor_decref when its set-to-null behavior is not
 * needed
 *
 * @param item Reference to an item
 */
CBOR_EXPORT void cbor_intermediate_decref(cbor_item_t* item);

/** Get the item's reference count
 *
 * \rst
 * .. warning:: This does *not* account for transitive references.
 * \endrst
 *
 * @todo Add some inline examples for reference counting
 *
 * @param item the item
 * @return the reference count
 */
_CBOR_NODISCARD
CBOR_EXPORT size_t cbor_refcount(const cbor_item_t* item);

/** Provides CPP-like move construct
 *
 * Decreases the reference count by one, but does not deallocate the item even
 * if its refcount reaches zero. This is useful for passing intermediate values
 * to functions that increase reference count. Should only be used with
 * functions that `incref` their arguments.
 *
 * \rst
 * .. warning:: If the item is moved without correctly increasing the reference
 *  count afterwards, the memory will be leaked.
 * \endrst
 *
 * @param item Reference to an item
 * @return the item with reference count decreased by one
 */
_CBOR_NODISCARD
CBOR_EXPORT cbor_item_t* cbor_move(cbor_item_t* item);

#ifdef __cplusplus
}
#endif

#endif  // LIBCBOR_COMMON_H
