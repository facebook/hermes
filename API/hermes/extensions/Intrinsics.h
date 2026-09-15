/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "jsi/jsi.h"

namespace facebook {
namespace hermes {

/// Holds references to built-in JavaScript constructors captured at extension
/// initialization time. This ensures extensions use the original intrinsics
/// even if user code replaces the global properties.
struct ExtensionIntrinsics {
  jsi::Function typeError;
  jsi::Function rangeError;
  jsi::Function uint8Array;

  /// Object.getOwnPropertyDescriptor and DataView.prototype, captured once so
  /// the DataView accessor getters below can be extracted without re-fetching
  /// them per getter. Declared before the getters so they are initialized
  /// first and can be reused by the getters' initializers.
  jsi::Function getOwnPropertyDescriptor;
  jsi::Object dataViewPrototype;

  /// DataView.prototype accessor getters, captured before user code runs so
  /// DataView detection cannot be subverted by replacing globals.
  jsi::Function dataViewBufferGetter;
  jsi::Function dataViewByteOffsetGetter;
  jsi::Function dataViewByteLengthGetter;

  /// Capture intrinsics from the runtime. Must be called before any user code
  /// executes.
  explicit ExtensionIntrinsics(jsi::Runtime &rt);
};

/// Capture and store intrinsics from the runtime. Must be called before any
/// user code executes.
void captureIntrinsics(jsi::Runtime &rt);

/// Retrieve the stored intrinsics. Throws if captureIntrinsics was not called.
const ExtensionIntrinsics &getIntrinsics(jsi::Runtime &rt);

/// Throw a TypeError with the given message. Uses the intrinsic TypeError
/// constructor captured at initialization time.
[[noreturn]] void throwTypeError(jsi::Runtime &rt, const char *message);

/// Throw a RangeError with the given message. Uses the intrinsic RangeError
/// constructor captured at initialization time.
[[noreturn]] void throwRangeError(jsi::Runtime &rt, const char *message);

/// \return true iff \p obj is a genuine DataView. Brand-checks via the
/// captured DataView.prototype.buffer getter (immune to shadowing and global
/// replacement). The buffer getter is used rather than byteLength/byteOffset
/// because it does not reject a genuine but detached DataView.
bool isDataView(jsi::Runtime &rt, const jsi::Object &obj);

/// \return the underlying ArrayBuffer of the DataView \p dv.
/// \pre isDataView(rt, dv) is true.
jsi::ArrayBuffer dataViewBuffer(jsi::Runtime &rt, const jsi::Object &dv);

/// \return the byteOffset of the DataView \p dv.
/// \pre isDataView(rt, dv) is true.
size_t dataViewByteOffset(jsi::Runtime &rt, const jsi::Object &dv);

/// \return the byteLength of the DataView \p dv.
/// \pre isDataView(rt, dv) is true.
size_t dataViewByteLength(jsi::Runtime &rt, const jsi::Object &dv);

} // namespace hermes
} // namespace facebook
