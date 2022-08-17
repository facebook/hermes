/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// dtoa doesn't provide a header file so this simple one was created.

#ifndef HERMES_DTOA_DTOA_H
#define HERMES_DTOA_DTOA_H

#ifdef __cplusplus
extern "C" {
#endif

/// dtoa functions need to allocate memory and that job is handled by the dtoa
/// allocator. \c dtoa_alloc is an opaque struct representing the allocator. It
/// is created by calling \c dtoa_alloc_init() with a pointer to a memory
/// buffer declared with \c DECL_DTOA_ALLOC_MEM(name, size).
///
/// The allocator metadata itself is placed in that memory buffer and the rest
/// of it is used to satisfy memory allocations. If it is not enough, additional
/// allocations are made in the regular heap with malloc()/free(). So, the
/// larger the buffer declared by \c DECL_DTOA_ALLOC_MEM(), the less probability
/// there that a heap allocation will be needed.
///
/// The allocator is not thread safe, so we must guarantee that it is used only
/// by one thread a time. Usually we just create it on the stack (which is very
/// fast), but it could live in other places, as long as the single thread
/// requirement is satisfied.
///
/// The allocator metadata itself is less than 128 bytes - the rest is the
/// "allocation buffer". The dtoa documentation states that 2304 byte allocation
/// buffer is sufficient for most cases except the unusual ones, and a 7400 byte
/// allocation buffer is sufficient for all cases.
///
/// We don't need to avoid heap allocation at all costs, so we have chosen a
/// total allocator size of 1200 bytes, which seems to avoid heap allocations
/// for "normal" cases.
typedef struct dtoa_alloc dtoa_alloc;

/// The minimal size of the dtoa allocator memory buffer. The metadata is less
/// than 128 bytes and the rest is available to satisfy dtoa allocations.
#define DTOA_ALLOC_MIN_SIZE 256
/// The default size of the dtoa allocator memory buffer, which we use when
/// declaring it on the stack. The value attempts to find balance between
/// excessive stack consumption and avoiding allocations for "normal" cases.
#define DTOA_ALLOC_DEFAULT_SIZE 1200

/// This macro is used to declare a memory buffer for the dtoa allocator. Most
/// of all it ensures that the memory is properly aligned. The variable declared
/// by this macro is then passed to \c dtoa_alloc_init().
#define DECL_DTOA_ALLOC_MEM(name, bytelen) \
  union {                                  \
    void *p;                               \
    double d;                              \
    long long l;                           \
    char mem[(bytelen)];                   \
  } name

/// Initialize an allocator using the specified memory buffer, and return a
/// pointer to the allocator. Note that this does not stipulate that the
/// returned pointer will equal \c mem.
dtoa_alloc *dtoa_alloc_init(void *mem, int bytelen);

/// Destroy the previously initialized allocator. Primarily, this call frees
/// any heap allocations in the allocator.
void dtoa_alloc_done(dtoa_alloc *dalloc);

/// Converts double into ascii string.
/// \param dd the double to convert.
/// \param mode the rounding mode, 0 for default.<ul>
///     <li>0 ==> shortest string that yields d when read in
///             and rounded to nearest.
///     <li>1 ==> like 0, but with Steele & White stopping rule;
///             e.g. with IEEE P754 arithmetic , mode 0 gives
///             1e23 whereas mode 1 gives 9.999999999999999e22.
///     <li>2 ==> max(1,ndigits) significant digits.  This gives a
///             return value similar to that of ecvt, except
///             that trailing zeros are suppressed.
///     <li>3 ==> through ndigits past the decimal point.  This
///             gives a return value similar to that from fcvt,
///             except that trailing zeros are suppressed, and
///             ndigits can be negative.
///     <li>4,5 ==> similar to 2 and 3, respectively, but (in
///             round-nearest mode) with the tests of mode 0 to
///             possibly return a shorter string that rounds to d.
///             With IEEE arithmetic and compilation with
///             -DHonor_FLT_ROUNDS, modes 4 and 5 behave the same
///             as modes 2 and 3 when FLT_ROUNDS != 1.
///     <li>6-9 ==> Debugging modes similar to mode - 4:  don't try
///             fast floating-point estimate (if applicable).
///     <li>Values of mode other than 0-9 are treated as mode 0.
/// </ul>
/// \param ndigits number of digits of precision, 0 for default.
/// \param decpt where to store position of the decimal. (n in ES5.1 9.8.1)
/// \param sign location to store 1 if negative number, 0 if positive number.
/// \param rve location to store pointer to the end of the returned string.
/// \return string representation of s in ES5.1 9.8.1
char *g_dtoa(
    dtoa_alloc *dalloc,
    double dd,
    int mode,
    int ndigits,
    int *decpt,
    int *sign,
    char **rve);

/// Same as dtoa, but #defines ROUND_BIASED, which enables the mode which is
/// used for getting results with a fixed number of digits after the decimal.
/// It also modifies a check in dtoa (see the NOTE in dtoa_fixed.c),
/// which ensures that 0.5 does not get flushed to 0, but rather rounds up to 1.
/// A separate function is necessary because dtoa needs compilation flags
/// to change options and provides no runtime means of doing so,
/// and modification of the code was needed to ensure correctly biased rounding.
char *dtoa_fixedpoint(
    dtoa_alloc *dalloc,
    double dd,
    int mode,
    int ndigits,
    int *decpt,
    int *sign,
    char **rve);

/// Free the result of \c g_dtoa() and \c dtoa_fixedpoint().
void g_freedtoa(dtoa_alloc *dalloc, char *);

char *g_fmt(char *, double);
double hermes_g_strtod(const char *s00, char **se);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
/// A convenience RAII wrapper around a dtoa allocator. The usage should be
/// self-explanatory. Declare it on the stack (or as a class member), supplying
/// the memory buffer size, and pass it to the dtoa functions.
template <int bytelen = DTOA_ALLOC_DEFAULT_SIZE>
class DtoaAllocator {
 public:
  DtoaAllocator(const DtoaAllocator &) = delete;
  void operator=(const DtoaAllocator &) = delete;

  DtoaAllocator() {
    dalloc_ = dtoa_alloc_init(&mem_, bytelen);
  }
  ~DtoaAllocator() {
    dtoa_alloc_done(dalloc_);
  }

  operator dtoa_alloc *() {
    return dalloc_;
  }

 private:
  DECL_DTOA_ALLOC_MEM(mem_, bytelen);
  dtoa_alloc *dalloc_;
};
#endif

#endif // HERMES_DTOA_DTOA_H
