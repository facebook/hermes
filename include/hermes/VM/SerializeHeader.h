/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_SERIALIZEHEADER_H
#define HERMES_VM_SERIALIZEHEADER_H

#include "hermes/Support/Compiler.h"

namespace hermes {
namespace vm {

/// Relocation kind. Used to distinguish different kind of pointers
enum class RelocationKind { NativePointer, GCPointer, HermesValue };

constexpr uint32_t SD_MAGIC = 0xad082463;

constexpr uint32_t SD_HEADER_VERSION = 1;

/// Bump this version number up whenever NativeFunctions.def is changed.
constexpr uint32_t NATIVE_FUNCTION_VERSION = 1;

/// Serialize data header. Used to sanity check serialize data and make sure
/// that serializer and deserializer are consistent.
struct SerializeHeader {
  uint32_t magic = SD_MAGIC;
  uint32_t version = SD_HEADER_VERSION;
  uint32_t nativeFunctionTableVersion = NATIVE_FUNCTION_VERSION;
  uint32_t maxHeapSize = 0;

  /// Set if debug build.
  bool isDebug = false;
  /// Set if HERMES_ENABLE_DEBUGGER.
  bool isEnableDebugger = false;
};

static_assert(
    std::is_standard_layout<SerializeHeader>::value,
    "SerializeHeader isn't a standard layout");
static_assert(
    IsTriviallyCopyable<SerializeHeader, true>::value,
    "SerializeHeader should be trivially copyable");

} // namespace vm
} // namespace hermes

#endif
