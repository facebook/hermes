/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IR_IRVERIFIER_H
#define HERMES_IR_IRVERIFIER_H

#include "hermes/IR/IR.h"
#include "llvh/Support/raw_ostream.h"

using llvh::raw_ostream;

namespace hermes {

class Module;

enum class VerificationMode {
  /// Validate that the IR is valid.
  IR_VALID,
  /// Validate that the IR optimized and ready for bytecode generation.
  IR_OPTIMIZED,
  /// Validate lowered IR.
  IR_LOWERED,
};

/// \brief Check a module for errors.
///
/// If an error is found, a message describing the error is written to OS
/// (if non-null) and false is returned.  Note that this function is a noop
/// (returning true) in opt and dev builds and is only fully implemented in dbg
/// builds due to impact on performance.
///
/// \return true if module is verified successfully, false otherwise.
bool verifyModule(
    const Module &M,
    raw_ostream *OS = nullptr,
    VerificationMode mode = VerificationMode::IR_VALID);

} // namespace hermes

#endif
