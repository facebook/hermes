/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "InternalBindings.h"
#include "hermes/hermes.h"

using namespace facebook;

/// Creates a new array buffer by calling the JS constructor. If
/// the buffer was not able to be constructed, throws an error.
static jsi::ArrayBuffer newArrayBuffer(jsi::Runtime &rt, size_t size) {
  jsi::Object retBufferAsObject = rt.global()
                                      .getPropertyAsFunction(rt, "ArrayBuffer")
                                      .callAsConstructor(rt, (int)size)
                                      .asObject(rt);
  if (!retBufferAsObject.isArrayBuffer(rt))
    throw jsi::JSError(rt, "Could not construct ArrayBuffer");

  return retBufferAsObject.getArrayBuffer(rt);
}

/// Returns the the utf8 length of a jsi::String that is passed in as a
/// jsi::Value. Note: this function is currently not needed by fs.js and
/// was primarily used for initial testing purposes.
static jsi::Value byteLengthUtf8(
    RuntimeState &rs,
    const jsi::Value &thisValue,
    const jsi::Value *args,
    size_t count) {
  jsi::Runtime &rt = rs.getRuntime();
  if (count < 1) {
    throw jsi::JSError(rt, "Not enough arguments passed in to byteLengthUtf8.");
  }
  if (!args[0].isString()) {
    throw jsi::JSError(rt, "byteLengthUtf8 must be called on a string object");
  }

  return jsi::Value(rt, (int)args[0].toString(rt).utf8(rt).length());
}

/// Given the caller jsi::Value, which should translate to a Buffer object,
/// returns a jsi::String representing the sliced version of the array.
static jsi::Value utf8Slice(
    RuntimeState &rs,
    const jsi::Value &thisValue,
    const jsi::Value *args,
    size_t count) {
  jsi::Runtime &rt = rs.getRuntime();
  jsi::Object thisBuffer =
      thisValue.asObject(rt).getPropertyAsObject(rt, "buffer");
  if (!thisBuffer.isArrayBuffer(rt)) {
    throw jsi::JSError(
        rt,
        "UTF8Slice must be called on an object that contains an arraybuffer.");
  }
  if (count < 2) {
    throw jsi::JSError(rt, "Not enough arguments passed in to utf8Slice.");
  }
  size_t start = args[0].asNumber();
  size_t len = args[1].asNumber();

  jsi::ArrayBuffer existingArray = thisBuffer.getArrayBuffer(rt);
  if (start + len > existingArray.size(rt)) {
    throw jsi::JSError(rt, "Index out of range");
  }

  return jsi::String::createFromUtf8(rt, existingArray.data(rt) + start, len);
}

/// Creates an ArrayBuffer the size of an uint32_t and returns it. Used for
/// internal buffer purposes. This is implemented in v8 with a private object.
static jsi::Value getZeroFillToggle(
    RuntimeState &rs,
    const jsi::Value &thisValue,
    const jsi::Value *args,
    size_t count) {
  jsi::Runtime &rt = rs.getRuntime();
  return newArrayBuffer(rt, sizeof(uint32_t));
}

/// Adds the 'buffer' object as a property of internalBinding.
/// Initializes the relevant internalBinding('buffer') functions to get
/// Buffer.alloc, Buffer.concat and Buffer.toString working in buffer.js.
jsi::Value facebook::bufferBinding(RuntimeState &rs) {
  jsi::Runtime &rt = rs.getRuntime();
  jsi::Object buffer{rt};

  rs.defineJSFunction(byteLengthUtf8, "byteLengthUtf8", 1, buffer);
  rs.defineJSFunction(getZeroFillToggle, "getZeroFillToggle", 0, buffer);
  rs.defineJSFunction(utf8Slice, "utf8Slice", 2, buffer);

  jsi::Value kMaxLength{(double)(uint64_t{2} << 32)};
  buffer.setProperty(rt, "kMaxLength", kMaxLength);

  rs.setInternalBindingProp("Buffer", std::move(buffer));
  return rs.getInternalBindingProp("Buffer");
}
