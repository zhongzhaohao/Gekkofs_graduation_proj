/**
 * Copyright (c) 2013-2021 UChicago Argonne, LLC and The HDF Group.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Generated file. Only edit mercury_config.h.in. */

#ifndef MERCURY_CONFIG_H
#define MERCURY_CONFIG_H

/*************************************/
/* Public Type and Struct Definition */
/*************************************/

/* Type definitions */
#ifdef _WIN32
typedef signed __int64 hg_int64_t;
typedef signed __int32 hg_int32_t;
typedef signed __int16 hg_int16_t;
typedef signed __int8 hg_int8_t;
typedef unsigned __int64 hg_uint64_t;
typedef unsigned __int32 hg_uint32_t;
typedef unsigned __int16 hg_uint16_t;
typedef unsigned __int8 hg_uint8_t;
/* Limits on Integer Constants */
#    define UINT64_MAX _UI64_MAX
#else
#    include <stddef.h>
#    include <stdint.h>
typedef int64_t hg_int64_t;
typedef int32_t hg_int32_t;
typedef int16_t hg_int16_t;
typedef int8_t hg_int8_t;
typedef uint64_t hg_uint64_t;
typedef uint32_t hg_uint32_t;
typedef uint16_t hg_uint16_t;
typedef uint8_t hg_uint8_t;
#endif
typedef hg_uint64_t hg_ptr_t;
typedef hg_uint8_t hg_bool_t;

/* True / false */
#define HG_TRUE  1
#define HG_FALSE 0

/*****************/
/* Public Macros */
/*****************/

/* Reflects major releases of Mercury */
#define HG_VERSION_MAJOR 2
/* Reflects any API changes */
#define HG_VERSION_MINOR 1
/* Reflects any library code changes */
#define HG_VERSION_PATCH 0

#include <mercury_compiler_attributes.h>

/* Inline macro */
#ifdef _WIN32
#    define HG_INLINE __inline
#else
#    define HG_INLINE __inline__
#endif

/* Fallthrough macro */
#define HG_FALLTHROUGH HG_ATTR_FALLTHROUGH

/* Shared libraries */
#define HG_BUILD_SHARED_LIBS
#ifdef HG_BUILD_SHARED_LIBS
#    ifdef mercury_EXPORTS
#        define HG_PUBLIC HG_ATTR_ABI_EXPORT
#    else
#        define HG_PUBLIC HG_ATTR_ABI_IMPORT
#    endif
#    define HG_PRIVATE HG_ATTR_ABI_HIDDEN
#else
#    define HG_PUBLIC
#    define HG_PRIVATE
#endif

/* Build Options */
#define HG_HAS_BOOST
/* #undef HG_HAS_CHECKSUMS */
/* #undef HG_HAS_XDR */
/* #undef HG_HAS_COLLECT_STATS */

/* #undef HG_HAS_DEBUG */

#endif /* MERCURY_CONFIG_H */
