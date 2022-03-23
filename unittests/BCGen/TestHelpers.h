/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_BCGEN_TESTHELPERS_H
#define HERMES_UNITTESTS_BCGEN_TESTHELPERS_H

#include "hermes/AST/Context.h"

#include <stdint.h>
#include <vector>
#include "llvh/ADT/SmallVector.h"

namespace hermes {

struct TestCompileFlags {
  bool staticBuiltins{false};
};

/// Compile source code \p source into Hermes bytecode, asserting that it can be
/// compiled successfully. \return the bytecode as a vector of bytes.
std::vector<uint8_t> bytecodeForSource(
    const char *source,
    TestCompileFlags flags = TestCompileFlags());

} // namespace hermes

#endif
