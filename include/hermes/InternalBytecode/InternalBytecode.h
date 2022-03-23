/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_INTERNALBYTECODE_INTERNALBYTECODE_H
#define HERMES_INTERNALBYTECODE_INTERNALBYTECODE_H

/// \file
/// InternalBytecode is a way to embed JS source into the VM at build time.
/// The purpose of this is to implement parts of Hermes in JS.
/// The build will concatenate multiple JS files together to form a single file,
/// which will be compiled and turned into a static C array of bytes.
/// Concatenating the files has multiple benefits:
///   * Explicit control of the initialization order
///   * Single string table and bytecode file
///   * Explicit control over scoping and sharing
///   * Cross-file inlining
///   * Cross-file inlining
///   * Better optimization because sharing occurs in local variables,
///     not in object properties.

#include "llvh/ADT/ArrayRef.h"

#include <cstdint>
#include <vector>

namespace hermes {
namespace vm {

/// Get a pre-compiled bytecode module to be included with the VM upon
/// construction. This module must be run before any user code can be run.
/// \return A list of bytes that can be turned into a Hermes bytecode module.
llvh::ArrayRef<uint8_t> getInternalBytecode();

} // namespace vm
} // namespace hermes

#endif // HERMES_INTERNALBYTECODE_INTERNALBYTECODE_H
