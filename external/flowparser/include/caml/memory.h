/**************************************************************************/
/*                                                                        */
/*                                 OCaml                                  */
/*                                                                        */
/*              Damien Doligez, projet Para, INRIA Rocquencourt           */
/*                                                                        */
/*   Copyright 1996 Institut National de Recherche en Informatique et     */
/*     en Automatique.                                                    */
/*                                                                        */
/*   All rights reserved.  This file is distributed under the terms of    */
/*   the GNU Lesser General Public License version 2.1, with the          */
/*   special exception on linking described in the file LICENSE.          */
/*                                                                        */
/**************************************************************************/

/* Allocation macros and functions */

#ifndef CAML_MEMORY_H
#define CAML_MEMORY_H

#ifndef CAML_NAME_SPACE
#include "compatibility.h"
#endif
#include "config.h"
#ifdef CAML_INTERNALS
#include "gc.h"
#include "major_gc.h"
#include "minor_gc.h"
#endif /* CAML_INTERNALS */
#include "misc.h"
#include "mlvalues.h"

#ifdef __cplusplus
extern "C" {
#endif


CAMLextern value caml_alloc_shr (mlsize_t wosize, tag_t);
#ifdef WITH_PROFINFO
CAMLextern value caml_alloc_shr_with_profinfo (mlsize_t, tag_t, intnat);
CAMLextern value caml_alloc_shr_preserving_profinfo (mlsize_t, tag_t,
                                                     header_t);
#else
#define caml_alloc_shr_with_profinfo(size, tag, profinfo) \
  caml_alloc_shr(size, tag)
#define caml_alloc_shr_preserving_profinfo(size, tag, header) \
  caml_alloc_shr(size, tag)
#endif /* WITH_PROFINFO */
CAMLextern value caml_alloc_shr_no_raise (mlsize_t wosize, tag_t);
CAMLextern void caml_adjust_gc_speed (mlsize_t, mlsize_t);
CAMLextern void caml_alloc_dependent_memory (mlsize_t bsz);
CAMLextern void caml_free_dependent_memory (mlsize_t bsz);
CAMLextern void caml_modify (value *, value);
CAMLextern void caml_initialize (value *, value);
CAMLextern value caml_check_urgent_gc (value);
CAMLextern void * caml_stat_alloc (asize_t);              /* Size in bytes. */
CAMLextern void caml_stat_free (void *);
CAMLextern void * caml_stat_resize (void *, asize_t);     /* Size in bytes. */
CAMLextern int caml_init_alloc_for_heap (void);
CAMLextern char *caml_alloc_for_heap (asize_t request);   /* Size in bytes. */
CAMLextern void caml_free_for_heap (char *mem);
CAMLextern void caml_disown_for_heap (char *mem);
CAMLextern int caml_add_to_heap (char *mem);
CAMLextern color_t caml_allocation_color (void *hp);

CAMLextern int caml_huge_fallback_count;

/* void caml_shrink_heap (char *);        Only used in compact.c */

#ifdef CAML_INTERNALS

extern uintnat caml_use_huge_pages;

#ifdef HAS_HUGE_PAGES
#include <sys/mman.h>
#define Heap_page_size HUGE_PAGE_SIZE
#define Round_mmap_size(x)                                      \
    (((x) + (Heap_page_size - 1)) & ~ (Heap_page_size - 1))
#endif


int caml_page_table_add(int kind, void * start, void * end);
int caml_page_table_remove(int kind, void * start, void * end);
int caml_page_table_initialize(mlsize_t bytesize);

#ifdef DEBUG
#define DEBUG_clear(result, wosize) do{ \
  uintnat caml__DEBUG_i; \
  for (caml__DEBUG_i = 0; caml__DEBUG_i < (wosize); ++ caml__DEBUG_i){ \
    Field ((result), caml__DEBUG_i) = Debug_uninit_minor; \
  } \
}while(0)
#else
#define DEBUG_clear(result, wosize)
#endif

#define Alloc_small_with_profinfo(result, wosize, tag, profinfo) do {       \
                                                CAMLassert ((wosize) >= 1); \
                                          CAMLassert ((tag_t) (tag) < 256); \
                                 CAMLassert ((wosize) <= Max_young_wosize); \
  caml_young_ptr -= Whsize_wosize (wosize);                                 \
  if (caml_young_ptr < caml_young_trigger){                                 \
    caml_young_ptr += Whsize_wosize (wosize);                               \
    CAML_INSTR_INT ("force_minor/alloc_small@", 1);                         \
    Setup_for_gc;                                                           \
    caml_gc_dispatch ();                                                    \
    Restore_after_gc;                                                       \
    caml_young_ptr -= Whsize_wosize (wosize);                               \
  }                                                                         \
  Hd_hp (caml_young_ptr) =                                                  \
    Make_header_with_profinfo ((wosize), (tag), Caml_black, profinfo);      \
  (result) = Val_hp (caml_young_ptr);                                       \
  DEBUG_clear ((result), (wosize));                                         \
}while(0)

#if defined(NATIVE_CODE) && defined(WITH_SPACETIME)
extern uintnat caml_spacetime_my_profinfo(struct ext_table**, uintnat);
#define Alloc_small(result, wosize, tag) \
  Alloc_small_with_profinfo(result, wosize, tag, \
    caml_spacetime_my_profinfo(NULL, wosize))
#else
#define Alloc_small(result, wosize, tag) \
  Alloc_small_with_profinfo(result, wosize, tag, (uintnat) 0)
#endif

/* Deprecated alias for [caml_modify] */

#define Modify(fp,val) caml_modify((fp), (val))

#endif /* CAML_INTERNALS */

struct caml__roots_block {
  struct caml__roots_block *next;
  intnat ntables;
  intnat nitems;
  value *tables [5];
};

CAMLextern struct caml__roots_block *caml_local_roots;  /* defined in roots.c */

/* The following macros are used to declare C local variables and
   function parameters of type [value].

   The function body must start with one of the [CAMLparam] macros.
   If the function has no parameter of type [value], use [CAMLparam0].
   If the function has 1 to 5 [value] parameters, use the corresponding
   [CAMLparam] with the parameters as arguments.
   If the function has more than 5 [value] parameters, use [CAMLparam5]
   for the first 5 parameters, and one or more calls to the [CAMLxparam]
   macros for the others.
   If the function takes an array of [value]s as argument, use
   [CAMLparamN] to declare it (or [CAMLxparamN] if you already have a
   call to [CAMLparam] for some other arguments).

   If you need local variables of type [value], declare them with one
   or more calls to the [CAMLlocal] macros at the beginning of the
   function, after the call to CAMLparam.  Use [CAMLlocalN] (at the
   beginning of the function) to declare an array of [value]s.

   Your function may raise an exception or return a [value] with the
   [CAMLreturn] macro.  Its argument is simply the [value] returned by
   your function.  Do NOT directly return a [value] with the [return]
   keyword.  If your function returns void, use [CAMLreturn0]. If you
   un-register the local roots (i.e. undo the effects of the [CAMLparam*]
   and [CAMLlocal] macros) without returning immediately, use [CAMLdrop].

   All the identifiers beginning with "caml__" are reserved by OCaml.
   Do not use them for anything (local or global variables, struct or
   union tags, macros, etc.)
*/

#define CAMLparam0() \
  struct caml__roots_block *caml__frame = caml_local_roots

#define CAMLparam1(x) \
  CAMLparam0 (); \
  CAMLxparam1 (x)

#define CAMLparam2(x, y) \
  CAMLparam0 (); \
  CAMLxparam2 (x, y)

#define CAMLparam3(x, y, z) \
  CAMLparam0 (); \
  CAMLxparam3 (x, y, z)

#define CAMLparam4(x, y, z, t) \
  CAMLparam0 (); \
  CAMLxparam4 (x, y, z, t)

#define CAMLparam5(x, y, z, t, u) \
  CAMLparam0 (); \
  CAMLxparam5 (x, y, z, t, u)

#define CAMLparamN(x, size) \
  CAMLparam0 (); \
  CAMLxparamN (x, (size))

/* CAMLunused is preserved for compatibility reasons.
   Instead of the legacy GCC/Clang-only
     CAMLunused foo;
   you should prefer
     CAMLunused_start foo CAMLunused_end;
   which supports both GCC/Clang and MSVC.
*/
#if defined(__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 7))
  #define CAMLunused_start __attribute__ ((unused))
  #define CAMLunused_end
  #define CAMLunused __attribute__ ((unused))
#elif _MSC_VER >= 1500
  #define CAMLunused_start  __pragma( warning (push) )           \
    __pragma( warning (disable:4189 ) )
  #define CAMLunused_end __pragma( warning (pop))
  #define CAMLunused
#else
  #define CAMLunused_start
  #define CAMLunused_end
  #define CAMLunused
#endif

#define CAMLxparam1(x) \
  struct caml__roots_block caml__roots_##x; \
  CAMLunused_start int caml__dummy_##x = ( \
    (void) caml__frame, \
    (caml__roots_##x.next = caml_local_roots), \
    (caml_local_roots = &caml__roots_##x), \
    (caml__roots_##x.nitems = 1), \
    (caml__roots_##x.ntables = 1), \
    (caml__roots_##x.tables [0] = &x), \
    0) \
   CAMLunused_end

#define CAMLxparam2(x, y) \
  struct caml__roots_block caml__roots_##x; \
  CAMLunused_start int caml__dummy_##x = ( \
    (void) caml__frame, \
    (caml__roots_##x.next = caml_local_roots), \
    (caml_local_roots = &caml__roots_##x), \
    (caml__roots_##x.nitems = 1), \
    (caml__roots_##x.ntables = 2), \
    (caml__roots_##x.tables [0] = &x), \
    (caml__roots_##x.tables [1] = &y), \
    0) \
   CAMLunused_end

#define CAMLxparam3(x, y, z) \
  struct caml__roots_block caml__roots_##x; \
  CAMLunused_start int caml__dummy_##x = ( \
    (void) caml__frame, \
    (caml__roots_##x.next = caml_local_roots), \
    (caml_local_roots = &caml__roots_##x), \
    (caml__roots_##x.nitems = 1), \
    (caml__roots_##x.ntables = 3), \
    (caml__roots_##x.tables [0] = &x), \
    (caml__roots_##x.tables [1] = &y), \
    (caml__roots_##x.tables [2] = &z), \
    0) \
  CAMLunused_end

#define CAMLxparam4(x, y, z, t) \
  struct caml__roots_block caml__roots_##x; \
  CAMLunused_start int caml__dummy_##x = ( \
    (void) caml__frame, \
    (caml__roots_##x.next = caml_local_roots), \
    (caml_local_roots = &caml__roots_##x), \
    (caml__roots_##x.nitems = 1), \
    (caml__roots_##x.ntables = 4), \
    (caml__roots_##x.tables [0] = &x), \
    (caml__roots_##x.tables [1] = &y), \
    (caml__roots_##x.tables [2] = &z), \
    (caml__roots_##x.tables [3] = &t), \
    0) \
  CAMLunused_end

#define CAMLxparam5(x, y, z, t, u) \
  struct caml__roots_block caml__roots_##x; \
  CAMLunused_start int caml__dummy_##x = ( \
    (void) caml__frame, \
    (caml__roots_##x.next = caml_local_roots), \
    (caml_local_roots = &caml__roots_##x), \
    (caml__roots_##x.nitems = 1), \
    (caml__roots_##x.ntables = 5), \
    (caml__roots_##x.tables [0] = &x), \
    (caml__roots_##x.tables [1] = &y), \
    (caml__roots_##x.tables [2] = &z), \
    (caml__roots_##x.tables [3] = &t), \
    (caml__roots_##x.tables [4] = &u), \
    0) \
  CAMLunused_end

#define CAMLxparamN(x, size) \
  struct caml__roots_block caml__roots_##x; \
  CAMLunused_start int caml__dummy_##x = (     \
    (void) caml__frame, \
    (caml__roots_##x.next = caml_local_roots), \
    (caml_local_roots = &caml__roots_##x), \
    (caml__roots_##x.nitems = (size)), \
    (caml__roots_##x.ntables = 1), \
    (caml__roots_##x.tables[0] = &(x[0])), \
    0) \
  CAMLunused_end

#define CAMLlocal1(x) \
  value x = Val_unit; \
  CAMLxparam1 (x)

#define CAMLlocal2(x, y) \
  value x = Val_unit, y = Val_unit; \
  CAMLxparam2 (x, y)

#define CAMLlocal3(x, y, z) \
  value x = Val_unit, y = Val_unit, z = Val_unit; \
  CAMLxparam3 (x, y, z)

#define CAMLlocal4(x, y, z, t) \
  value x = Val_unit, y = Val_unit, z = Val_unit, t = Val_unit; \
  CAMLxparam4 (x, y, z, t)

#define CAMLlocal5(x, y, z, t, u) \
  value x = Val_unit, y = Val_unit, z = Val_unit, t = Val_unit, u = Val_unit; \
  CAMLxparam5 (x, y, z, t, u)

#define CAMLlocalN(x, size) \
  value x [(size)]; \
  int caml__i_##x; \
  for (caml__i_##x = 0; caml__i_##x < size; caml__i_##x ++) { \
    x[caml__i_##x] = Val_unit; \
  } \
  CAMLxparamN (x, (size))


#define CAMLdrop caml_local_roots = caml__frame

#define CAMLreturn0 do{ \
  CAMLdrop; \
  return; \
}while (0)

#define CAMLreturnT(type, result) do{ \
  type caml__temp_result = (result); \
  CAMLdrop; \
  return caml__temp_result; \
}while(0)

#define CAMLreturn(result) CAMLreturnT(value, result)

#define CAMLnoreturn ((void) caml__frame)


/* convenience macro */
#define Store_field(block, offset, val) do{ \
  mlsize_t caml__temp_offset = (offset); \
  value caml__temp_val = (val); \
  caml_modify (&Field ((block), caml__temp_offset), caml__temp_val); \
}while(0)

/*
   NOTE: [Begin_roots] and [End_roots] are superseded by [CAMLparam]*,
   [CAMLxparam]*, [CAMLlocal]*, [CAMLreturn].

   [Begin_roots] and [End_roots] are used for C variables that are GC roots.
   It must contain all values in C local variables and function parameters
   at the time the minor GC is called.
   Usage:
   After initialising your local variables to legal OCaml values, but before
   calling allocation functions, insert [Begin_roots_n(v1, ... vn)], where
   v1 ... vn are your variables of type [value] that you want to be updated
   across allocations.
   At the end, insert [End_roots()].

   Note that [Begin_roots] opens a new block, and [End_roots] closes it.
   Thus they must occur in matching pairs at the same brace nesting level.

   You can use [Val_unit] as a dummy initial value for your variables.
*/

#define Begin_root Begin_roots1

#define Begin_roots1(r0) { \
  struct caml__roots_block caml__roots_block; \
  caml__roots_block.next = caml_local_roots; \
  caml_local_roots = &caml__roots_block; \
  caml__roots_block.nitems = 1; \
  caml__roots_block.ntables = 1; \
  caml__roots_block.tables[0] = &(r0);

#define Begin_roots2(r0, r1) { \
  struct caml__roots_block caml__roots_block; \
  caml__roots_block.next = caml_local_roots; \
  caml_local_roots = &caml__roots_block; \
  caml__roots_block.nitems = 1; \
  caml__roots_block.ntables = 2; \
  caml__roots_block.tables[0] = &(r0); \
  caml__roots_block.tables[1] = &(r1);

#define Begin_roots3(r0, r1, r2) { \
  struct caml__roots_block caml__roots_block; \
  caml__roots_block.next = caml_local_roots; \
  caml_local_roots = &caml__roots_block; \
  caml__roots_block.nitems = 1; \
  caml__roots_block.ntables = 3; \
  caml__roots_block.tables[0] = &(r0); \
  caml__roots_block.tables[1] = &(r1); \
  caml__roots_block.tables[2] = &(r2);

#define Begin_roots4(r0, r1, r2, r3) { \
  struct caml__roots_block caml__roots_block; \
  caml__roots_block.next = caml_local_roots; \
  caml_local_roots = &caml__roots_block; \
  caml__roots_block.nitems = 1; \
  caml__roots_block.ntables = 4; \
  caml__roots_block.tables[0] = &(r0); \
  caml__roots_block.tables[1] = &(r1); \
  caml__roots_block.tables[2] = &(r2); \
  caml__roots_block.tables[3] = &(r3);

#define Begin_roots5(r0, r1, r2, r3, r4) { \
  struct caml__roots_block caml__roots_block; \
  caml__roots_block.next = caml_local_roots; \
  caml_local_roots = &caml__roots_block; \
  caml__roots_block.nitems = 1; \
  caml__roots_block.ntables = 5; \
  caml__roots_block.tables[0] = &(r0); \
  caml__roots_block.tables[1] = &(r1); \
  caml__roots_block.tables[2] = &(r2); \
  caml__roots_block.tables[3] = &(r3); \
  caml__roots_block.tables[4] = &(r4);

#define Begin_roots_block(table, size) { \
  struct caml__roots_block caml__roots_block; \
  caml__roots_block.next = caml_local_roots; \
  caml_local_roots = &caml__roots_block; \
  caml__roots_block.nitems = (size); \
  caml__roots_block.ntables = 1; \
  caml__roots_block.tables[0] = (table);

#define End_roots() caml_local_roots = caml__roots_block.next; }


/* [caml_register_global_root] registers a global C variable as a memory root
   for the duration of the program, or until [caml_remove_global_root] is
   called. */

CAMLextern void caml_register_global_root (value *);

/* [caml_remove_global_root] removes a memory root registered on a global C
   variable with [caml_register_global_root]. */

CAMLextern void caml_remove_global_root (value *);

/* [caml_register_generational_global_root] registers a global C
   variable as a memory root for the duration of the program, or until
   [caml_remove_generational_global_root] is called.
   The program guarantees that the value contained in this variable
   will not be assigned directly.  If the program needs to change
   the value of this variable, it must do so by calling
   [caml_modify_generational_global_root].  The [value *] pointer
   passed to [caml_register_generational_global_root] must contain
   a valid OCaml value before the call.
   In return for these constraints, scanning of memory roots during
   minor collection is made more efficient. */

CAMLextern void caml_register_generational_global_root (value *);

/* [caml_remove_generational_global_root] removes a memory root
   registered on a global C variable with
   [caml_register_generational_global_root]. */

CAMLextern void caml_remove_generational_global_root (value *);

/* [caml_modify_generational_global_root(r, newval)]
   modifies the value contained in [r], storing [newval] inside.
   In other words, the assignment [*r = newval] is performed,
   but in a way that is compatible with the optimized scanning of
   generational global roots.  [r] must be a global memory root
   previously registered with [caml_register_generational_global_root]. */

CAMLextern void caml_modify_generational_global_root(value *r, value newval);

#ifdef __cplusplus
}
#endif

#endif /* CAML_MEMORY_H */
