/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Intrinsics.h"

namespace facebook {
namespace hermes {
namespace {

/// UUID for storing ExtensionIntrinsics in the runtime.
/// Generated randomly, must be unique across all runtime data users.
constexpr jsi::UUID kIntrinsicsUUID{
    0x8a3f7b2c,
    0x4d1e,
    0x4a5f,
    0x9c8b,
    0x0000'd2e1f3a4b5c6};

/// Return the getter function of the accessor property \p name on \p proto,
/// obtained via the genuine \p getOwnPropertyDescriptor. Capturing the original
/// getter lets extensions brand-check objects by their internal slots
/// regardless of later tampering with the global prototype or descriptor
/// functions. Called only during capture, before user code runs.
jsi::Function capturePrototypeGetter(
    jsi::Runtime &rt,
    const jsi::Function &getOwnPropertyDescriptor,
    const jsi::Object &proto,
    const char *name) {
  return getOwnPropertyDescriptor
      .call(rt, proto, jsi::String::createFromAscii(rt, name))
      .asObject(rt)
      .getPropertyAsFunction(rt, "get");
}

} // namespace

ExtensionIntrinsics::ExtensionIntrinsics(jsi::Runtime &rt)
    : typeError(rt.global().getPropertyAsFunction(rt, "TypeError")),
      rangeError(rt.global().getPropertyAsFunction(rt, "RangeError")),
      uint8Array(rt.global().getPropertyAsFunction(rt, "Uint8Array")),
      getOwnPropertyDescriptor(
          rt.global()
              .getPropertyAsObject(rt, "Object")
              .getPropertyAsFunction(rt, "getOwnPropertyDescriptor")),
      dataViewPrototype(rt.global()
                            .getPropertyAsObject(rt, "DataView")
                            .getPropertyAsObject(rt, "prototype")),
      dataViewBufferGetter(capturePrototypeGetter(
          rt,
          getOwnPropertyDescriptor,
          dataViewPrototype,
          "buffer")),
      dataViewByteOffsetGetter(capturePrototypeGetter(
          rt,
          getOwnPropertyDescriptor,
          dataViewPrototype,
          "byteOffset")),
      dataViewByteLengthGetter(capturePrototypeGetter(
          rt,
          getOwnPropertyDescriptor,
          dataViewPrototype,
          "byteLength")) {}

void captureIntrinsics(jsi::Runtime &rt) {
  auto intrinsics = std::make_shared<ExtensionIntrinsics>(rt);
  rt.setRuntimeData(kIntrinsicsUUID, intrinsics);
}

const ExtensionIntrinsics &getIntrinsics(jsi::Runtime &rt) {
  auto data = rt.getRuntimeData(kIntrinsicsUUID);
  if (!data) {
    throw jsi::JSINativeException(
        "Extension intrinsics not initialized. "
        "captureIntrinsics() must be called before using extensions.");
  }
  return *static_cast<ExtensionIntrinsics *>(data.get());
}

[[noreturn]] void throwTypeError(jsi::Runtime &rt, const char *message) {
  const auto &intrinsics = getIntrinsics(rt);
  jsi::Value error =
      intrinsics.typeError.call(rt, jsi::String::createFromUtf8(rt, message));
  throw jsi::JSError(rt, std::move(error));
}

[[noreturn]] void throwRangeError(jsi::Runtime &rt, const char *message) {
  const auto &intrinsics = getIntrinsics(rt);
  jsi::Value error =
      intrinsics.rangeError.call(rt, jsi::String::createFromUtf8(rt, message));
  throw jsi::JSError(rt, std::move(error));
}

bool isDataView(jsi::Runtime &rt, const jsi::Object &obj) {
  const auto &intrinsics = getIntrinsics(rt);
  try {
    // The buffer getter throws a TypeError unless `obj` has a genuine
    // [[DataView]] internal slot. Unlike the byteLength/byteOffset getters, it
    // does NOT throw for a genuine but detached DataView, so detachment is
    // reported later as a detached-buffer error instead of misclassifying the
    // object as "not a DataView".
    intrinsics.dataViewBufferGetter.callWithThis(rt, obj);
    return true;
  } catch (const jsi::JSError &) {
    return false;
  }
}

jsi::ArrayBuffer dataViewBuffer(jsi::Runtime &rt, const jsi::Object &dv) {
  const auto &intrinsics = getIntrinsics(rt);
  return intrinsics.dataViewBufferGetter.callWithThis(rt, dv)
      .asObject(rt)
      .getArrayBuffer(rt);
}

size_t dataViewByteOffset(jsi::Runtime &rt, const jsi::Object &dv) {
  const auto &intrinsics = getIntrinsics(rt);
  return static_cast<size_t>(
      intrinsics.dataViewByteOffsetGetter.callWithThis(rt, dv).asNumber());
}

size_t dataViewByteLength(jsi::Runtime &rt, const jsi::Object &dv) {
  const auto &intrinsics = getIntrinsics(rt);
  return static_cast<size_t>(
      intrinsics.dataViewByteLengthGetter.callWithThis(rt, dv).asNumber());
}

} // namespace hermes
} // namespace facebook
