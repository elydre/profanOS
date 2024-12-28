/*****************************************************************************\
|   === cdefs.h : 2024 ===                                                    |
|                                                                             |
|    Implementation of the cdefs.h header file from libC           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original code from openbsd (see below)                        `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

/* == Copyright (c) 1991, 1993
 * The Regents of the University of California. All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Berkeley Software Design, Inc.
 *
 * https://github.com/openbsd/src/blob/master/sys/sys/cdefs.h
 */

#ifndef _SYS_CDEFS_H
#define _SYS_CDEFS_H

// Macro to test if we're using a specific version of gcc or later

#ifdef __GNUC__
  #define __GNUC_PREREQ__(ma, mi) \
        ((__GNUC__ > (ma)) || (__GNUC__ == (ma) && __GNUC_MINOR__ >= (mi)))
#else
  #define __GNUC_PREREQ__(ma, mi) 0
#endif


/*
 * The __CONCAT macro is used to concatenate parts of symbol names, e.g.
 * with "#define OLD(foo) __CONCAT(old,foo)", OLD(foo) produces oldfoo.
 * The __CONCAT macro is a bit tricky -- make sure you don't put spaces
 * in between its arguments.  Do not use __CONCAT on double-quoted strings,
 * such as those from the __STRING macro: to concatenate strings just put
 * them next to each other.
 */

#if defined(__STDC__) || defined(__cplusplus)
  #define __P(protos)   protos      // full-blown ANSI C
  #define __CONCAT(x,y) x ## y
  #define __STRING(x)   #x

  #define __const         const     // define reserved names to standard
  #define __signed        signed
  #define __volatile      volatile

  #if defined(__cplusplus) || defined(__PCC__)
    #define __inline    inline      // convert to C++ keyword
  #else
    #if !defined(__GNUC__)
      #define __inline              // delete GCC keyword
    #endif
  #endif

#else
  #define __P(protos)   ()          // traditional C preprocessor
  #define __CONCAT(x,y) x/**/y
  #define __STRING(x)   "x"

  #if !defined(__GNUC__)
    #define __const
    #define __inline
    #define __signed
    #define __volatile
  #endif
#endif

/*
 * GCC1 and some versions of GCC2 declare dead (non-returning) and
 * pure (no side effects) functions using "volatile" and "const";
 * unfortunately, these then cause warnings under "-ansi -pedantic".
 * GCC >= 2.5 uses the __attribute__((attrs)) style.  All of these
 * work for GNU C++ (modulo a slight glitch in the C++ grammar in
 * the distribution version of 2.5.5).
 */

#if !__GNUC_PREREQ__(2, 5) && !defined(__PCC__)
  #define __attribute__(x)    // delete __attribute__ if non-gcc or gcc1
  #if defined(__GNUC__) && !defined(__STRICT_ANSI__)
    #define __dead  __volatile
    #define __pure  __const
  #endif
#else
  #define __dead    __attribute__((__noreturn__))
  #define __pure    __attribute__((__const__))
#endif

#if __GNUC_PREREQ__(2, 7)
  #define __unused  __attribute__((__unused__))
#else
  #define __unused
#endif

#if __GNUC_PREREQ__(3, 1)
  #define __used    __attribute__((__used__))
#else
  #define __used    __unused    // suppress -Wunused warnings
#endif

#if __GNUC_PREREQ__(3,4)
  #define __warn_unused_result  __attribute__((__warn_unused_result__))
#else
  #define __warn_unused_result
#endif

#if __GNUC_PREREQ__(3,3) && !defined(__clang__)
  #define __bounded(args)   __attribute__ ((__bounded__ args ))
#else
  #define __bounded(args)
#endif

/*
 * __returns_twice makes the compiler not assume the function
 * only returns once. This affects registerisation of variables:
 * even local variables need to be in memory across such a call.
 * Example: setjmp()
 */

#if __GNUC_PREREQ__(4, 1)
  #define __returns_twice __attribute__((returns_twice))
#else
  #define __returns_twice
#endif

/*
 * __only_inline makes the compiler only use this function definition
 * for inlining; references that can't be inlined will be left as
 * external references instead of generating a local copy.  The
 * matching library should include a simple extern definition for
 * the function to handle those references.  c.f. ctype.h
 */

#ifdef __GNUC__
  #if __GNUC_PREREQ__(4, 2)
    #define __only_inline extern __inline __attribute__((__gnu_inline__))
  #else
    #define __only_inline extern __inline
  #endif
#else
  #define __only_inline static __inline
#endif

/*
 * GNU C version 2.96 adds explicit branch prediction so that
 * the CPU back-end can hint the processor and also so that
 * code blocks can be reordered such that the predicted path
 * sees a more linear flow, thus improving cache behavior, etc.
 *
 * The following two macros provide us with a way to utilize this
 * compiler feature.  Use __predict_true() if you expect the expression
 * to evaluate to true, and __predict_false() if you expect the
 * expression to evaluate to false.
 *
 * A few notes about usage:
 *
 *  * Generally, __predict_false() error condition checks (unless
 *    you have some _strong_ reason to do otherwise, in which case
 *    document it), and/or __predict_true() `no-error' condition
 *    checks, assuming you want to optimize for the no-error case.
 *
 *  * Other than that, if you don't know the likelihood of a test
 *    succeeding from empirical or other `hard' evidence, don't
 *    make predictions.
 *
 *  * These are meant to be used in places that are run `a lot'.
 *    It is wasteful to make predictions in code that is run
 *    seldomly (e.g. at subsystem initialization time) as the
 *    basic block reordering that this affects can often generate
 *    larger code.
 */

#if __GNUC_PREREQ__(2, 96)
  #define __predict_true(exp)   __builtin_expect(((exp) != 0), 1)
  #define __predict_false(exp)  __builtin_expect(((exp) != 0), 0)
#else
  #define __predict_true(exp)   ((exp) != 0)
  #define __predict_false(exp)  ((exp) != 0)
#endif

// Delete pseudo-keywords wherever they are not available or needed
#ifndef __dead
  #define __dead
  #define __pure
#endif

/*
 * The __packed macro indicates that a variable or structure members
 * should have the smallest possible alignment, despite any host CPU
 * alignment requirements.
 *
 * The __aligned(x) macro specifies the minimum alignment of a
 * variable or structure.
 *
 * These macros together are useful for describing the layout and
 * alignment of messages exchanged with hardware or other systems.
 */

#if __GNUC_PREREQ__(2, 7) || defined(__PCC__)
  #define __packed      __attribute__((__packed__))
  #define __aligned(x)  __attribute__((__aligned__(x)))
#endif

#if !__GNUC_PREREQ__(2, 8)
  #define __extension__
#endif

#if __GNUC_PREREQ__(3, 0)
  #define __malloc  __attribute__((__malloc__))
#else
  #define __malloc
#endif

#if defined(__cplusplus)
  #define __BEGIN_EXTERN_C    extern "C" {
  #define __END_EXTERN_C      }
#else
  #define __BEGIN_EXTERN_C
  #define __END_EXTERN_C
#endif

#if __GNUC_PREREQ__(4, 0)
  #define __dso_public __attribute__((__visibility__("default")))
  #define __dso_hidden __attribute__((__visibility__("hidden")))

  #define __BEGIN_PUBLIC_DECLS \
        _Pragma("GCC visibility push(default)") __BEGIN_EXTERN_C
  #define __BEGIN_HIDDEN_DECLS \
        _Pragma("GCC visibility push(hidden)") __BEGIN_EXTERN_C

  #define __END_PUBLIC_DECLS __END_EXTERN_C _Pragma("GCC visibility pop")
  #define __END_HIDDEN_DECLS __END_EXTERN_C _Pragma("GCC visibility pop")
#else
  #define __dso_public
  #define __dso_hidden
  #define __BEGIN_PUBLIC_DECLS    __BEGIN_EXTERN_C
  #define __BEGIN_HIDDEN_DECLS    __BEGIN_EXTERN_C
  #define __END_PUBLIC_DECLS      __END_EXTERN_C
  #define __END_HIDDEN_DECLS      __END_EXTERN_C
#endif

#define __BEGIN_DECLS   __BEGIN_EXTERN_C
#define __END_DECLS     __END_EXTERN_C

#undef __GNUC_PREREQ__

#endif
