/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TextEncoder.h"

#include "Intrinsics.h"

#include "hermes/Support/UTF8.h"
#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/Optional.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <type_traits>

namespace facebook {
namespace hermes {
namespace {

/// NativeState subclass used to identify TextEncoder instances.
class TextEncoderNativeState : public jsi::NativeState {
 public:
  TextEncoderNativeState() = default;
};

/// MutableBuffer implementation that owns a std::string of UTF-8 data.
class UTF8Buffer : public jsi::MutableBuffer {
 public:
  explicit UTF8Buffer(std::string data) : data_(std::move(data)) {}

  size_t size() const override {
    return data_.size();
  }

  uint8_t *data() override {
    return reinterpret_cast<uint8_t *>(data_.data());
  }

 private:
  std::string data_;
};

/// Validate that 'thisVal' is a TextEncoder instance (has our NativeState).
/// Throws a JSError if not.
inline void validateTextEncoder(
    jsi::Runtime &rt,
    const jsi::Value &thisVal,
    const char *methodName) {
  if (!thisVal.isObject() ||
      !thisVal.asObject(rt).hasNativeState<TextEncoderNativeState>(rt)) {
    throw jsi::JSError(
        rt, std::string(methodName) + " called on non-TextEncoder object");
  }
}

/// Result of getTypedArrayBuffer - the destination buffer info.
struct TypedArrayBufferInfo {
  uint8_t *data;
  size_t byteLength;
};

/// Try to convert a jsi::Value to a valid unsigned integer of type T.
/// Returns llvh::None if the value is not a number, is NaN, negative,
/// not an integer, exceeds MAX_SAFE_INTEGER, or exceeds the range of T.
template <typename T>
inline llvh::Optional<T> valueToUnsigned(const jsi::Value &val) {
  static_assert(std::is_unsigned<T>::value, "T must be an unsigned type");
  if (!val.isNumber()) {
    return llvh::None;
  }
  double d = val.asNumber();
  // Check non-negative and within MAX_SAFE_INTEGER (2^53 - 1), which is exactly
  // representable as a double. This ensures d could be an exact integer.
  constexpr double maxSafeInteger = 9007199254740991.0; // 2^53 - 1
  if (!(d >= 0 && d <= maxSafeInteger)) {
    return llvh::None;
  }
  // Cast to uint64_t and verify no fractional part was lost.
  auto u = static_cast<uint64_t>(d);
  if (static_cast<double>(u) != d) {
    return llvh::None;
  }
  if (u > std::numeric_limits<T>::max()) {
    return llvh::None;
  }
  return static_cast<T>(u);
}

/// Extract buffer info from a TypedArray-like object.
/// We use duck-typing (checking for buffer/byteOffset/byteLength) rather than
/// instanceof Uint8Array. This is more permissive than the spec requires, but
/// handles cross-realm Uint8Arrays and is consistent with JSI's portable
/// design. The original VM implementation used internal type checks.
/// Throws JSError if the object is not a valid TypedArray or is detached.
inline TypedArrayBufferInfo getTypedArrayBuffer(
    jsi::Runtime &rt,
    const jsi::Value &val) {
  if (!val.isObject()) {
    throw jsi::JSError(rt, "The second argument should be a Uint8Array");
  }
  jsi::Object obj = val.asObject(rt);

  // Get the underlying ArrayBuffer via the 'buffer' property
  jsi::Value bufferVal = obj.getProperty(rt, "buffer");
  if (!bufferVal.isObject() || !bufferVal.asObject(rt).isArrayBuffer(rt)) {
    throw jsi::JSError(rt, "The second argument should be a Uint8Array");
  }
  jsi::ArrayBuffer arrayBuffer = bufferVal.asObject(rt).getArrayBuffer(rt);

  // Get byteOffset and byteLength from the TypedArray view
  llvh::Optional<size_t> byteOffset =
      valueToUnsigned<size_t>(obj.getProperty(rt, "byteOffset"));
  llvh::Optional<size_t> byteLength =
      valueToUnsigned<size_t>(obj.getProperty(rt, "byteLength"));
  if (!byteOffset || !byteLength) {
    throw jsi::JSError(rt, "The second argument should be a Uint8Array");
  }

  // Get raw pointer. data(rt) throws JSINativeException if the buffer is
  // detached, so we catch it and throw a proper TypeError.
  uint8_t *bufferData;
  try {
    bufferData = arrayBuffer.data(rt);
  } catch (const jsi::JSINativeException &) {
    throwTypeError(
        rt,
        "TextEncoder.prototype.encodeInto called on a detached ArrayBuffer");
  }

  return {bufferData + *byteOffset, *byteLength};
}

/// Native helper called by the JS constructor to install NativeState.
/// This marks the object as a valid TextEncoder instance.
jsi::Value textEncoderInit(
    jsi::Runtime &rt,
    const jsi::Value &,
    const jsi::Value *args,
    size_t count) {
  // Defensive check: we expect exactly 1 argument from our JS constructor.
  if (count != 1) {
    throw jsi::JSError(rt, "textEncoderInit requires exactly 1 argument");
  }
  jsi::Object self = args[0].asObject(rt);
  self.setNativeState(rt, std::make_shared<TextEncoderNativeState>());
  return jsi::Value::undefined();
}

/// Implementation of TextEncoder.prototype.encode()
/// Encodes a string to UTF-8 and returns a Uint8Array.
jsi::Value textEncoderEncode(
    jsi::Runtime &rt,
    const jsi::Value &thisVal,
    const jsi::Value *args,
    size_t count) {
  validateTextEncoder(rt, thisVal, "TextEncoder.prototype.encode()");

  // Get Uint8Array constructor from intrinsics
  const jsi::Function &uint8ArrayCtor = getIntrinsics(rt).uint8Array;

  // Get input string (default to empty string if no argument)
  if (count == 0 || args[0].isUndefined()) {
    return uint8ArrayCtor.callAsConstructor(rt, 0);
  }

  // Convert to UTF-8. jsi::String::utf8() already handles invalid surrogates
  // with U+FFFD replacement, which is correct per the WHATWG Encoding Standard.
  std::string utf8 = args[0].toString(rt).utf8(rt);

  if (utf8.empty()) {
    return uint8ArrayCtor.callAsConstructor(rt, 0);
  }

  // Create a buffer holding the UTF-8 data
  auto buffer = std::make_shared<UTF8Buffer>(std::move(utf8));

  // Create ArrayBuffer from the buffer
  jsi::ArrayBuffer arrayBuffer(rt, buffer);

  // Call new Uint8Array(arrayBuffer)
  return uint8ArrayCtor.callAsConstructor(rt, std::move(arrayBuffer));
}

/// Implementation of TextEncoder.prototype.encodeInto()
/// Encodes a string into an existing Uint8Array and returns {read, written}.
jsi::Value textEncoderEncodeInto(
    jsi::Runtime &rt,
    const jsi::Value &thisVal,
    const jsi::Value *args,
    size_t count) {
  validateTextEncoder(rt, thisVal, "TextEncoder.prototype.encodeInto()");

  if (count < 2) {
    throw jsi::JSError(rt, "TextEncoder.encodeInto requires 2 arguments");
  }

  jsi::String inputStr = args[0].toString(rt);

  // Get the destination buffer info (validates and extracts
  // buffer/offset/length)
  TypedArrayBufferInfo destInfo = getTypedArrayBuffer(rt, args[1]);

  uint8_t *destStart = destInfo.data;
  uint32_t numRead = 0;
  uint32_t numWritten = 0;
  size_t remainingBytes = destInfo.byteLength;

  // Use getStringData to access internal representation efficiently.
  // ASCII strings can be copied directly (ASCII is a subset of UTF-8).
  // UTF-16 strings need conversion.
  auto cb = [&](bool ascii, const void *data, size_t num) {
    if (remainingBytes == 0 || num == 0) {
      return;
    }

    if (ascii) {
      // ASCII is a strict subset of UTF-8, copy directly
      size_t toCopy = std::min(num, remainingBytes);
      std::memcpy(destStart + numWritten, data, toCopy);
      numRead += static_cast<uint32_t>(toCopy);
      numWritten += static_cast<uint32_t>(toCopy);
      remainingBytes -= toCopy;
    } else {
      // UTF-16 data, convert to UTF-8 directly into the buffer
      llvh::ArrayRef<char16_t> utf16Data(
          static_cast<const char16_t *>(data), num);
      std::pair<uint32_t, uint32_t> result =
          ::hermes::convertUTF16ToUTF8BufferWithReplacements(
              llvh::MutableArrayRef<uint8_t>(
                  destStart + numWritten, remainingBytes),
              utf16Data);
      numRead += result.first;
      numWritten += result.second;
      remainingBytes -= result.second;
    }
  };
  inputStr.getStringData(rt, cb);

  // Return {read, written} object
  jsi::Object resultObj(rt);
  resultObj.setProperty(rt, "read", static_cast<double>(numRead));
  resultObj.setProperty(rt, "written", static_cast<double>(numWritten));
  return resultObj;
}

} // namespace

void installTextEncoder(jsi::Runtime &rt, jsi::Object &extensions) {
  // Get the setup function from the precompiled extensions object
  jsi::Function setup = extensions.getPropertyAsFunction(rt, "TextEncoder");

  // Create native helper functions
  jsi::Function nativeInit = jsi::Function::createFromHostFunction(
      rt, jsi::PropNameID::forAscii(rt, "textEncoderInit"), 1, textEncoderInit);

  jsi::Function nativeEncode = jsi::Function::createFromHostFunction(
      rt,
      jsi::PropNameID::forAscii(rt, "textEncoderEncode"),
      1,
      textEncoderEncode);

  jsi::Function nativeEncodeInto = jsi::Function::createFromHostFunction(
      rt,
      jsi::PropNameID::forAscii(rt, "textEncoderEncodeInto"),
      2,
      textEncoderEncodeInto);

  // Call the setup function with our native helpers
  setup.call(rt, nativeInit, nativeEncode, nativeEncodeInto);
}

} // namespace hermes
} // namespace facebook
