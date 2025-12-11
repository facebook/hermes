/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSIUtils.h"

namespace facebook {
namespace hermes {

TypedArrayBufferInfo getTypedArrayBuffer(
    jsi::Runtime &rt,
    const jsi::Value &val,
    const char *errorMessage,
    const char *detachedErrorMessage) {
  if (!val.isObject()) {
    throw jsi::JSError(rt, errorMessage);
  }
  jsi::Object obj = val.asObject(rt);

  // Get the underlying ArrayBuffer via the 'buffer' property
  jsi::Value bufferVal = obj.getProperty(rt, "buffer");
  if (!bufferVal.isObject() || !bufferVal.asObject(rt).isArrayBuffer(rt)) {
    throw jsi::JSError(rt, errorMessage);
  }
  jsi::ArrayBuffer arrayBuffer = bufferVal.asObject(rt).getArrayBuffer(rt);

  // Get byteOffset and byteLength from the TypedArray view
  llvh::Optional<size_t> byteOffset =
      valueToUnsigned<size_t>(obj.getProperty(rt, "byteOffset"));
  llvh::Optional<size_t> byteLength =
      valueToUnsigned<size_t>(obj.getProperty(rt, "byteLength"));
  if (!byteOffset || !byteLength) {
    throw jsi::JSError(rt, errorMessage);
  }

  // Get raw pointer. data(rt) throws JSINativeException if the buffer is
  // detached, so we catch it and throw a proper TypeError.
  uint8_t *bufferData;
  try {
    bufferData = arrayBuffer.data(rt);
  } catch (const jsi::JSINativeException &) {
    throwTypeError(rt, detachedErrorMessage);
  }

  return {bufferData + *byteOffset, *byteLength};
}

} // namespace hermes
} // namespace facebook
