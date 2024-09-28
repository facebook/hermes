/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_HERMES_SAFE_MATH_H
#define HERMES_SUPPORT_HERMES_SAFE_MATH_H

#include "hermes/Support/ErrorHandling.h"

#include <cassert>
#include <limits>
#include <type_traits>

/// NOTE: Some of the functions in this file produce error messages via
/// static asserts.  The expected form of such asserts is checked via
///
///   test/static_asserts/test_static_asserts.sh
///
/// which must be run manually (for various reasons).
/// So: IF YOU MAKE CHANGES IN THIS FILE, PLEASE RUN THAT TEST.

namespace hermes {

/// Preconditions for cast-like operations.  Requires \p FromType and
/// \p ToType to both be integral types.  We require them to have the
/// same signed-ness, to avoid the confusions associated with converting
/// between signed and unsigned types.  Finally, we require that \p FromType
/// is at least as wide as \p ToType -- that the operation is not a
/// widening.  (We don't require \p FromType to be strictly wider because
/// some type aliases, like size_t, have different definitions in different
/// contexts.  So we only require the conversion to be a *possible*
/// narrowing.)
template <typename ToType, typename FromType>
constexpr void narrowingPreconditions() {
  // We only support integral types.
  static_assert(std::is_integral<FromType>::value, "FromType must be integral");
  static_assert(std::is_integral<ToType>::value, "ToType must be integral");
  // Don't check the remaining conditions if the two above aren't met.
  if constexpr (
      std::is_integral<FromType>::value && std::is_integral<ToType>::value) {
    // Avoid confusing signed/unsigned conversions.
    static_assert(
        std::is_unsigned<FromType>::value == std::is_unsigned<ToType>::value,
        "Signed-ness of FromType and ToType must agree; signed-unsigned "
        "conversions are error-prone.");
    static_assert(
        sizeof(FromType) >= sizeof(ToType),
        "Should only use when conversion is (possibly) narrowing");
  }
}

/// Statically determines whether a conversion from \p FromType to
/// \p ToType is narrowing.  If so, does a dynamic overflow check on
/// \p val, calling hermes_fatal with \p msg if this fails.
/// Requires \p FromType and \p ToType to obey the narrowingPreconditions: that
/// they are both integral, have the same signed-ness, and that \p FromType
/// is at least as wide as \p ToType.
template <typename ToType, typename FromType>
void sizeCheck(FromType val, const char *msg) {
  narrowingPreconditions<ToType, FromType>();
  if constexpr (sizeof(FromType) > sizeof(ToType)) {
    if (LLVM_UNLIKELY(val != static_cast<FromType>(static_cast<ToType>(val)))) {
      hermes_fatal(msg);
    }
  }
}

/// If \p FromType is wider than \p ToType, performs an overflow check
/// on \p from, calling hermes_fatal with \p msg if \p from is not
/// representable in \p ToType. Otherwise, returns \p from cast to \p ToType.
/// Requires \p FromType and \p ToType to obey the narrowingPreconditions: that
/// they are both integral, have the same signed-ness, and that \p FromType
/// is at least as wide as \p ToType.
template <typename ToType, typename FromType>
ToType safePossiblyNarrowingCast(FromType from, const char *msg) {
  sizeCheck<ToType, FromType>(from, msg);
  return static_cast<ToType>(from);
}

/// Casts \p from to \p ToType, suppressing any associated compiler errors.
/// Requires \p FromType and \p ToType to obey the narrowingPreconditions: that
/// they are both integral, the same signed-ness, and that \p FromType is at
/// least as wide as \p ToType.
///
/// Every use of this requires a justification comment, explaining why it is
/// actually safe.
template <typename ToType, typename FromType>
constexpr ToType unsafeNarrow(FromType from) {
  narrowingPreconditions<ToType, FromType>();
  // The use of a static_cast suppresses any loss-of-precision
  // warning.  If, in the future, we implement a linter to find
  // all such casts, and we convert them to unsafeNarrow calls,
  // then we would need a directive here to disable the linter
  // on this instance.
  return static_cast<ToType>(from);
}

/// Cast from double to an integer type, when it is known ahead of time that
/// the cast is safe (the value is in range). This just marks the cast as such
/// and ensures that it is in debug mode.
template <typename ToType>
ToType ubcastFromDouble(double x) {
  static_assert(std::is_integral<ToType>::value, "ToType must be integral");
  assert(
      std::numeric_limits<ToType>::min() <= x &&
      x <= std::numeric_limits<ToType>::max() &&
      "Double value is not in range");
  return (ToType)x;
}

} // namespace hermes

#endif // HERMES_SUPPORT_HERMES_SAFE_MATH_H
