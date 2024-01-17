/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"

#include "hermes/ADT/SafeInt.h"
#include "hermes/VM/JSLib/Base64Util.h"
#include "hermes/VM/StringBuilder.h"

namespace hermes {
namespace vm {

/// Create a Base64-encoded ASCII string from an input string expected to have
/// each character in the range of U+0000 to U+00FF. Error is thrown if any
/// character is outside of the expected range.
CallResult<HermesValue> btoa(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};
  auto res = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto string = runtime.makeHandle(std::move(*res));

  // Figure out the expected encoded length
  uint64_t expectedLength = ((string->getStringLength() + 2) / 3) * 4;
  bool overflow = expectedLength > std::numeric_limits<uint32_t>::max();
  if (overflow) {
    return runtime.raiseError("String length to convert to base64 is too long");
  }
  SafeUInt32 outputLength{static_cast<uint32_t>(expectedLength)};
  CallResult<StringBuilder> builder =
      StringBuilder::createStringBuilder(runtime, outputLength, true);
  if (LLVM_UNLIKELY(builder == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  bool success = string->isASCII()
      ? base64Encode(string->getStringRef<char>(), *builder)
      : base64Encode(string->getStringRef<char16_t>(), *builder);
  if (!success) {
    return runtime.raiseError(
        "Found invalid character when converting to base64");
  }

  return builder->getStringPrimitive().getHermesValue();
}

} // namespace vm
} // namespace hermes
