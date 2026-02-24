/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_MD5_H
#define HERMES_SUPPORT_MD5_H

#include "llvh/ADT/ArrayRef.h"
#include "llvh/Support/MD5.h"

namespace hermes {

/// \return The MD5 checksum for a function, with function id \p funcId, and
/// bytecode stream \p bytecode.
llvh::MD5::MD5Result doMD5Checksum(
    uint32_t funcId,
    llvh::ArrayRef<uint8_t> bytecode);

} // namespace hermes

#endif // HERMES_SUPPORT_MD5_H
