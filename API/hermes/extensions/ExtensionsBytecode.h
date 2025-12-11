/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_EXTENSIONS_EXTENSIONSBYTECODE_H
#define HERMES_EXTENSIONS_EXTENSIONSBYTECODE_H

#include "llvh/ADT/ArrayRef.h"

#include <cstdint>

namespace facebook {
namespace hermes {

/// Returns the precompiled bytecode for JSI extensions.
/// This bytecode, when executed, returns an object with setup functions
/// keyed by extension name.
llvh::ArrayRef<uint8_t> getExtensionsBytecode();

} // namespace hermes
} // namespace facebook

#endif // HERMES_EXTENSIONS_EXTENSIONSBYTECODE_H
