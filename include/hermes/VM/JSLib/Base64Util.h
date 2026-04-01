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
#include "hermes/VM/StringView.h"

#include "llvh/ADT/SmallVector.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// Base64 Encoding (shared by btoa and Uint8Array.prototype.toBase64)
//===----------------------------------------------------------------------===//

/// Encode \p str to base64 characters and store the output in \p builder.
/// Used by both the WHATWG btoa() and TC39 Uint8Array.prototype.toBase64.
/// \p useBase64url if true, use the base64url alphabet (-_ instead of +/).
/// \p omitPadding if true, do not emit '=' padding characters.
/// \return true if successful, false otherwise
template <typename T>
bool base64Encode(
    llvh::ArrayRef<T> str,
    StringBuilder &builder,
    bool useBase64url = false,
    bool omitPadding = false);

//===----------------------------------------------------------------------===//
// WHATWG Forgiving Base64 Decode (used by atob)
// https://infra.spec.whatwg.org/#forgiving-base64-decode
//===----------------------------------------------------------------------===//

/// If \p str has a valid base64 encoded string length, then calculate the
/// expected length after decoding using the forgiving base64 algorithm. Returns
/// nullopt if \p str has an invalid length.
template <typename T>
OptValue<uint32_t> base64DecodeOutputLength(llvh::ArrayRef<T> str);

/// Implements the forgiving base64 decode algorithm.
/// The key difference compared to other base64 decode algorithms is that the
/// forgiving algorithm ignores whitespaces.
/// \param str string to be decoded
/// \param builder StringBuilder to store the output in
/// \return true if successful, false otherwise
template <typename T>
bool base64Decode(llvh::ArrayRef<T> str, StringBuilder &builder);

//===----------------------------------------------------------------------===//
// TC39 FromBase64 Algorithm (used by Uint8Array base64 methods)
// https://tc39.es/proposal-arraybuffer-base64/spec/#sec-frombase64
//===----------------------------------------------------------------------===//

/// How to handle the final incomplete chunk in base64 decoding.
enum class LastChunkHandling { Loose, Strict, StopBeforePartial };

/// Result of the TC39 FromBase64 operation.
struct Base64DecodeResult {
  /// Number of input characters consumed.
  uint32_t read;
  /// The decoded bytes.
  llvh::SmallVector<uint8_t, 64> bytes;
  /// Whether an error occurred during decoding.
  bool error;
  /// Index of the character that caused the error (valid only if error==true).
  uint32_t errorIndex;

  Base64DecodeResult() : read(0), error(false), errorIndex(0) {}
};

/// Implements the TC39 FromBase64 abstract operation (Section 10.3).
/// Unlike the WHATWG forgiving-base64 algorithm used by atob(), this supports:
/// - base64url alphabet (useBase64url)
/// - Three lastChunkHandling modes (loose/strict/stop-before-partial)
/// - maxLength to limit decoded output (for setFromBase64)
/// - Tracking of consumed input position (read) for partial decoding
/// - Non-zero padding bit handling (strict mode rejects, loose ignores)
///
/// \p input the input string view.
/// \p useBase64url whether to use the base64url alphabet.
/// \p lastChunkHandling how to handle the final partial chunk.
/// \p maxLength maximum number of bytes to decode.
Base64DecodeResult fromBase64(
    StringView input,
    bool useBase64url,
    LastChunkHandling lastChunkHandling,
    uint32_t maxLength);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSLIB_BASE64UTIL_H
