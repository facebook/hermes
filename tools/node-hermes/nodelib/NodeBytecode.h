/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef NODEHERMES_NODELIB_NODEBYTECODE_H
#define NODEHERMES_NODELIB_NODEBYTECODE_H

/// \file
/// NodeBytecode is a way to embed JS source modules needed to execute relevant
/// features in the node-hermes tool at build time.
/// Currently, the build concatenates multiple JS files together to form a
/// single file, which will be compiled and turned into a static C array
/// of bytes.

#include "llvh/ADT/ArrayRef.h"

#include <cstdint>
#include <vector>

/// Get a pre-compiled bytecode module to be included with node-hermes upon
/// construction.
/// \return A list of bytes that can be turned into a bytecode module.
llvh::ArrayRef<uint8_t> getNodeBytecode();

#endif // NODEHERMES_NODELIB_NODEBYTECODE_H
