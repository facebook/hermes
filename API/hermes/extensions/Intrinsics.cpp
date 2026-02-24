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

} // namespace

ExtensionIntrinsics::ExtensionIntrinsics(jsi::Runtime &rt)
    : typeError(rt.global().getPropertyAsFunction(rt, "TypeError")),
      rangeError(rt.global().getPropertyAsFunction(rt, "RangeError")),
      uint8Array(rt.global().getPropertyAsFunction(rt, "Uint8Array")) {}

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

} // namespace hermes
} // namespace facebook
