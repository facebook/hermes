/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_COMPILER_H
#define HERMES_SUPPORT_COMPILER_H

#include "llvh/Support/Compiler.h"

#include <type_traits>

// This file provides portable definitions of compiler specific macros
// It is modelled after LLVM's Compiler.h

// Some compilers will warn about unused variables, unless those variable types
// have a nontrivial constructor or destructor. This is due to types like
// std::lock_guard, whose variables are typically instantiated but not used.
// HERMES_ATTRIBUTE_WARN_UNUSED_VARIABLES  may be used to opt into unused
// variable warnings for types that would otherwise suppress them.
// Note this is orthogonal to warn_unused_result, which is a function attribute
// concerned with the return value of functions.
#if __has_attribute(warn_unused) || LLVM_GNUC_PREREQ(4, 9, 0)
#define HERMES_ATTRIBUTE_WARN_UNUSED_VARIABLES __attribute__((warn_unused))
#else
#define HERMES_ATTRIBUTE_WARN_UNUSED_VARIABLES
#endif

// Support for a type-level attribute declaring that, if you call a function
// that return this type, you should use it.
// This is equivalent to adding warn_unused_result to every function that
// returns this type.
// At the moment only clang supports this as a type-level attribute.
#if __has_attribute(warn_unused_result) && defined(__clang__)
#define HERMES_ATTRIBUTE_WARN_UNUSED_RESULT_TYPE \
  __attribute__((warn_unused_result))
#else
#define HERMES_ATTRIBUTE_WARN_UNUSED_RESULT_TYPE
#endif

#if __has_attribute(format) && defined(__clang__)
#define HERMES_ATTRIBUTE_FORMAT(archetype, string_index, first_to_check) \
  __attribute__((format(archetype, string_index, first_to_check)))
#else
#define HERMES_ATTRIBUTE_FORMAT(archetype, string_index, first_to_check)
#endif

#ifndef LLVM_PTR_SIZE
#error "LLVM_PTR_SIZE needs to be defined"
#endif

// In fbcode we might have bigger code samples during development due to
// integration testing. However, we're also at liberty to increase stack
// size at the application level and for things like HaaS we do this.
#if !defined(HERMES_FBCODE_BUILD) && !defined(HERMES_LARGE_STACK_DEPTH) && \
    (defined(HERMES_UBSAN) || LLVM_ADDRESS_SANITIZER_BUILD)
#define HERMES_LIMIT_STACK_DEPTH
#endif

#if LLVM_THREAD_SANITIZER_BUILD
// tsan detects these exact functions by name.
#ifdef __cplusplus
extern "C" {
#endif
void AnnotateIgnoreReadsBegin(const char *file, int line);
void AnnotateIgnoreReadsEnd(const char *file, int line);
void AnnotateIgnoreWritesBegin(const char *file, int line);
void AnnotateIgnoreWritesEnd(const char *file, int line);
void AnnotateBenignRaceSized(
    const char *file,
    int line,
    const volatile void *address,
    size_t size,
    const char *description);
void AnnotateThreadName(const char *file, int line, const char *name);
#ifdef __cplusplus
}
#endif

/// Ignore any races on reads between here and the next TsanIgnoreReadsEnd.
#define TsanIgnoreReadsBegin() AnnotateIgnoreReadsBegin(__FILE__, __LINE__)

/// Resume checking for racy reads.
#define TsanIgnoreReadsEnd() AnnotateIgnoreReadsEnd(__FILE__, __LINE__)

/// Ignore any races on writes between here and the next TsanIgnoreWritesEnd.
#define TsanIgnoreWritesBegin() AnnotateIgnoreWritesBegin(__FILE__, __LINE__)

/// Resume checking for racy writes.
#define TsanIgnoreWritesEnd() AnnotateIgnoreWritesEnd(__FILE__, __LINE__)

/// \def TsanBenignRaceSized(address, size, description)
/// Tell TSAN to ignore a race that was detected at the given address.
/// \param size Ignore a race on this many bytes after and including \p address.
/// \param description A string that will be shown in TSAN diagnostics to
///   explain why the race is benign.
#define TsanBenignRaceSized(address, size, description) \
  AnnotateBenignRaceSized(__FILE__, __LINE__, address, size, description)

/// Give the current thread a different name in TSAN output.
#define TsanThreadName(name) AnnotateThreadName(__FILE__, __LINE__, name)
#else
#define TsanIgnoreReadsBegin()
#define TsanIgnoreReadsEnd()
#define TsanBenignRaceSized(address, size, description)
#define TsanThreadName(name)
#endif

namespace hermes {

/// Convert from an l-value to an r-value. This is needed when we want to pass
/// a member constant to something that takes a const & - for example
/// std::max(). The compiler insists on passing a real address and on
/// instantiating the constant as a real variable, which, while possible, is
/// wasteful and very inconvenient.
template <typename T>
T toRValue(T x) {
  return x;
}

} // namespace hermes

#endif // HERMES_SUPPORT_COMPILER_H
