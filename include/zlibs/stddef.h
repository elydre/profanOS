/* Copyright (C) 2022-2024 mintsuki and contributors.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#if !defined(__FSTD_HDRS_STDDEF_H) || defined(__FSTD_HDRS_CXX_WRAP)
#if !defined(__FSTD_HDRS_CXX_WRAP)
#define __FSTD_HDRS_STDDEF_H 1
#endif

typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;

#ifndef __cplusplus
typedef __WCHAR_TYPE__ wchar_t;

/* XXX fix to use proper __STDC_VERSION__ number later on */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202000L)
typedef typeof(nullptr) nullptr_t;
#endif

#endif

#ifdef __cplusplus
typedef decltype(nullptr) nullptr_t;
#endif

#undef NULL
#ifndef __cplusplus
#  define NULL ((void *)0)
#else
#  define NULL 0
#endif

#undef offsetof
#define offsetof(s, m) __builtin_offsetof(s, m)

/* XXX fix to use proper __STDC_VERSION__ number later on */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202000L)
#  undef unreachable
#  define unreachable() __builtin_unreachable()
#endif

#endif
