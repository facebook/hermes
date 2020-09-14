//===-------------- lib/Support/Hashing.cpp -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides implementation bits for the LLVM common hashing
// infrastructure. Documentation and most of the other information is in the
// header file.
//
//===----------------------------------------------------------------------===//

#include "llvh/ADT/Hashing.h"

using namespace llvh;

// Provide a definition and static initializer for the fixed seed. This
// initializer should always be zero to ensure its value can never appear to be
// non-zero, even during dynamic initialization.
uint64_t llvh::hashing::detail::fixed_seed_override = 0;

// Implement the function for forced setting of the fixed seed.
// FIXME: Use atomic operations here so that there is no data race.
void llvh::set_fixed_execution_hash_seed(uint64_t fixed_value) {
  hashing::detail::fixed_seed_override = fixed_value;
}
