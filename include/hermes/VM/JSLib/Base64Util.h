/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSLIB_BASE64UTIL_H
#define HERMES_VM_JSLIB_BASE64UTIL_H

#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

/// Encode \p str to base64 characters and store the output in \p builder.
/// \return true if successful, false otherwise
template <typename T>
bool base64Encode(llvh::ArrayRef<T> str, StringBuilder &builder);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSLIB_BASE64UTIL_H
