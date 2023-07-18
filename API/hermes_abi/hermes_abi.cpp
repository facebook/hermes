/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_abi/hermes_abi.h"

#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"
#include "hermes/Public/RuntimeConfig.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Runtime.h"

using namespace hermes;
using namespace facebook::hermes;

/// A thin wrapper around vm::Runtime to provide additional state for things
/// like pointer management. It is intended to provide a small number of helper
/// functions, with the core logic being kept in the actual API functions below,
/// which can directly manipulate the vm::Runtime.
struct HermesABIContext {
  std::shared_ptr<::hermes::vm::Runtime> rt;

  explicit HermesABIContext(const hermes::vm::RuntimeConfig &runtimeConfig)
      : rt(hermes::vm::Runtime::create(runtimeConfig)) {}
};

HermesABIContext *make_hermes_runtime() {
  return new HermesABIContext({});
}

void release_hermes_runtime(HermesABIContext *runtime) {
  delete runtime;
}

extern "C" const HermesABIVTable *get_hermes_abi_vtable() {
  static const HermesABIVTable abiVtable = {
      make_hermes_runtime,
      release_hermes_runtime,
  };
  return &abiVtable;
}
