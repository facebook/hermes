/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_COMPILER_H
#define HERMES_SUPPORT_COMPILER_H

#include "hermes/Support/Config.h"

#include "llvm/Support/Compiler.h"

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

namespace hermes {

/// Some compiler versions don't support \c std::is_trivially_copyable<>, so
/// we are forced to abstract it away. A conservative default value must be
/// provided for the case when the compiler doesn't support it.
template <typename T, bool defaultValue>
struct IsTriviallyCopyable {
#ifdef HAVE_IS_TRIVIALLY_COPYABLE
  static constexpr bool value = std::is_trivially_copyable<T>::value;
#else
  static constexpr bool value = defaultValue;
#endif
};

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
