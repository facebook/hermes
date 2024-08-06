/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/HermesSafeMath.h"

using hermes::narrowingPreconditions;

namespace {

// clang-format off

void narrowingPreconditionsFromTypeIntegral() {
  narrowingPreconditions<uint64_t, double>();
  // CHECK:{{.*}}/Support/HermesSafeMath.h:{{[0-9]+}}:{{[0-9]+}}: error: static assertion failed due to requirement 'std::is_integral<double>::value': FromType must be integral
  // CHECK:[[@LINE-2]]:3: note: in instantiation of function template specialization 'hermes::narrowingPreconditions<unsigned long long, double>' requested here
}

void narrowingPreconditionsToTypeIntegral() {
  narrowingPreconditions<double, int64_t>();
  // CHECK:{{.*}}/Support/HermesSafeMath.h:{{[0-9]+}}:{{[0-9]+}}: error: static assertion failed due to requirement 'std::is_integral<double>::value': ToType must be integral
  // CHECK:[[@LINE-2]]:3: note: in instantiation of function template specialization 'hermes::narrowingPreconditions<double, long long>' requested here
}

void narrowingPreconditionsSignednessMismatch() {
  narrowingPreconditions<uint64_t, int64_t>();
  // CHECK:{{.*}}/Support/HermesSafeMath.h:{{[0-9]+}}:{{[0-9]+}}: error: static assertion failed due to requirement 'std::is_unsigned<long long>::value == std::is_unsigned<unsigned long long>::value': Signed-ness of FromType and ToType must agree; signed-unsigned conversions are error-prone.
  // CHECK:[[@LINE-2]]:3: note: in instantiation of function template specialization 'hermes::narrowingPreconditions<unsigned long long, long long>' requested here
}

void narrowingPreconditionsNotNarrowing() {
  narrowingPreconditions<uint64_t, uint32_t>();
  // CHECK:{{.*}}/Support/HermesSafeMath.h:{{[0-9]+}}:{{[0-9]+}}: error: static assertion failed due to requirement 'sizeof(unsigned int) >= sizeof(unsigned long long)': Should only use when conversion is (possibly) narrowing
  // CHECK:[[@LINE-2]]:3: note: in instantiation of function template specialization 'hermes::narrowingPreconditions<unsigned long long, unsigned int>' requested here
}

void testUnsafeNarrow(uint64_t wide) {
  // TODO: We would like to compile this with the flags that make narrowing conversions
  // errors, to demonstrate that we get narrowing/ conversion errors in the case below,
  // but not when using hermes::unsafeNarrow.  But that will have to wait until we
  // silence the errors in llvh.  (This is why the first line below is followed by an
  // "XXX_CHECK" comment, rather than a "CHECK" comment -- we don't expect that error
  // message now, but should in the future.)

  // Demonstrate that we get errors for narrowing conversions.
  uint32_t narrow0 = wide;
  // XXX_CHECK:[[@LINE-1]]:22: error: implicit conversion loses integer precision: 'uint64_t' (aka 'unsigned long long') to 'uint32_t' (aka 'unsigned int') [-Werror,-Wshorten-64-to-32]
  uint32_t narrow1 = hermes::unsafeNarrow<uint32_t>(wide);
  // No error on this line, though.
}

// clang-format on

} // namespace
