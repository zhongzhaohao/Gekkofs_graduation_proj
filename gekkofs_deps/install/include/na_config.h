/**
 * Copyright (c) 2013-2021 UChicago Argonne, LLC and The HDF Group.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Generated file. Only edit na_config.h.in. */

#ifndef NA_CONFIG_H
#define NA_CONFIG_H

/*************************************/
/* Public Type and Struct Definition */
/*************************************/

/* Type definitions */
#ifdef _WIN32
typedef signed __int64 na_int64_t;
typedef signed __int32 na_int32_t;
typedef signed __int16 na_int16_t;
typedef signed __int8 na_int8_t;
typedef unsigned __int64 na_uint64_t;
typedef unsigned __int32 na_uint32_t;
typedef unsigned __int16 na_uint16_t;
typedef unsigned __int8 na_uint8_t;
#else
#    include <stddef.h>
#    include <stdint.h>
typedef int64_t na_int64_t;
typedef int32_t na_int32_t;
typedef int16_t na_int16_t;
typedef int8_t na_int8_t;
typedef uint64_t na_uint64_t;
typedef uint32_t na_uint32_t;
typedef uint16_t na_uint16_t;
typedef uint8_t na_uint8_t;
#endif
typedef na_uint8_t na_bool_t;
typedef na_uint64_t na_ptr_t;

/* True / false */
#define NA_TRUE  1
#define NA_FALSE 0

/*****************/
/* Public Macros */
/*****************/

#include <mercury_compiler_attributes.h>

/* Unused return values */
#define NA_WARN_UNUSED_RESULT HG_ATTR_WARN_UNUSED_RESULT

/* Remove warnings when plugin does not use callback arguments */
#define NA_UNUSED HG_ATTR_UNUSED

/* Alignment */
#define NA_ALIGNED(x, a) HG_ATTR_ALIGNED(x, a)

/* Fallthrough */
#define NA_FALLTHROUGH HG_ATTR_FALLTHROUGH

/* Inline */
#ifdef _WIN32
#    define NA_INLINE __inline
#else
#    define NA_INLINE __inline__
#endif

/* Shared libraries */
#define NA_BUILD_SHARED_LIBS
#ifdef NA_BUILD_SHARED_LIBS
#    ifdef na_EXPORTS
#        define NA_PUBLIC HG_ATTR_ABI_EXPORT
#    else
#        define NA_PUBLIC HG_ATTR_ABI_IMPORT
#    endif
#    define NA_PRIVATE HG_ATTR_ABI_HIDDEN
#else
#    define NA_PUBLIC
#    define NA_PRIVATE
#endif

/* Build Options */
#define NA_HAS_MULTI_PROGRESS
/* #undef NA_HAS_DEBUG */

/* BMI */
/* #undef NA_HAS_BMI */

/* MPI */
/* #undef NA_HAS_MPI */
/* #undef NA_MPI_HAS_GNI_SETUP */

/* CCI */
/* #undef NA_HAS_CCI */

/* OFI */
#define NA_HAS_OFI
/* #undef NA_OFI_HAS_EXT_GNI_H */
/* #undef NA_OFI_GNI_HAS_UDREG */

/* NA SM */
#define NA_HAS_SM
/* #undef NA_SM_HAS_UUID */
#define NA_SM_HAS_CMA
#define NA_SM_SHM_PREFIX "na_sm"
#define NA_SM_TMP_DIRECTORY "/tmp"

/* UCX */
/* #undef NA_HAS_UCX */
/* #undef NA_UCX_HAS_LIB_QUERY */
/* #undef NA_UCX_HAS_THREAD_MODE_NAMES */

#endif /* NA_CONFIG_H */
