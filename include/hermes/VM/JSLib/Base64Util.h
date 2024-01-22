/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSLIB_BASE64UTIL_H
#define HERMES_VM_JSLIB_BASE64UTIL_H

#include "hermes/Support/OptValue.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

/// Encode \p str to base64 characters and store the output in \p builder.
/// \return true if successful, false otherwise
template <typename T>
bool base64Encode(llvh::ArrayRef<T> str, StringBuilder &builder);

/// If \p str has a valid base64 encoded string length, then calculate the
/// expected length after decoding using the forgiving base64 algorithm. Returns
/// nullopt if \p str has an invalid length.
template <typename T>
OptValue<uint32_t> base64DecodeOutputLength(llvh::ArrayRef<T> str);

/// Implements the forgiving base64 decode algorithm:
/// https://infra.spec.whatwg.org/#forgiving-base64-decode
/// The key difference compared to other base64 decode algorithms is that the
/// forgiving algorithm ignores whitespaces.
/// \param str string to be decoded
/// \param builder StringBuilder to store the output in
/// \return true if successful, false otherwise
template <typename T>
bool base64Decode(llvh::ArrayRef<T> str, StringBuilder &builder);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSLIB_BASE64UTIL_H
