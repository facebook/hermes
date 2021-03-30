/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SERIALIZEHEADER_H
#define HERMES_VM_SERIALIZEHEADER_H

#ifdef HERMESVM_SERIALIZE
#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/Support/Compiler.h"
#include "hermes/VM/NativeFunctions.def"

namespace hermes {
namespace vm {

/// Relocation kind. Used to distinguish different kind of pointers
enum class RelocationKind {
  NativePointer,
  GCPointer,
  HermesValue,
  SmallHermesValue
};

constexpr uint32_t SD_MAGIC = 0xad082463;

constexpr uint32_t SD_HEADER_VERSION = 2;

constexpr uint32_t NATIVE_FUNCTION_VERSION = NATIVE_FUNCTION_VERSION_VALUE;

/// Serialize data header. Used to sanity check serialize data and make sure
/// that serializer and deserializer are consistent.
struct SerializeHeader {
  uint32_t magic = SD_MAGIC;
  uint32_t version = SD_HEADER_VERSION;
  uint32_t nativeFunctionTableVersion = NATIVE_FUNCTION_VERSION;
  uint32_t heapSize = 0;

  /// Set if debug build.
  bool isDebug = false;
  /// Set if HERMES_ENABLE_DEBUGGER.
  bool isEnableDebugger = false;

  /// Runtime const fields. S/D works under the assumption that Serialize
  /// Runtime has the same runtime config as the Deserialize system. Write those
  /// flags that affects S/D in the header so we can check them too.
  bool enableEval;
  bool hasES6Promise;
  bool hasES6Proxy;
  bool hasIntl;
  uint8_t bytecodeWarmupPercent;
  bool trackIO;
  /// Note: The following fields are not being checked right now because they
  /// should't affect the correctness of the current stage of S/D:
  /// const bool verifyEvalIR;
  /// const bool shouldRandomizeMemoryLayout_;
  /// const uint8_t bytecodeWarmupPercent_;
  /// experiments::VMExperimentFlags vmExperimentFlags;
};

static_assert(
    std::is_standard_layout<SerializeHeader>::value,
    "SerializeHeader isn't a standard layout");
static_assert(
    IsTriviallyCopyable<SerializeHeader, true>::value,
    "SerializeHeader should be trivially copyable");

using ExternalPointersVectorFunction = std::vector<void *>();

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SERIALIZE
#endif
