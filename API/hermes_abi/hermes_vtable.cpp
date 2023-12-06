/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_abi/hermes_vtable.h"

#include "hermes_abi/hermes_abi.h"

#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"
#include "hermes/Public/RuntimeConfig.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Runtime.h"

#include "llvh/Support/ConvertUTF.h"

using namespace hermes;
using namespace facebook::hermes;

namespace {

/// A thin wrapper around vm::Runtime to provide additional state for things
/// like pointer management. It is intended to provide a small number of helper
/// functions, with the core logic being kept in the actual API functions below,
/// which can directly manipulate the vm::Runtime.
class HermesABIRuntimeImpl : public HermesABIRuntime {
  static const HermesABIRuntimeVTable vtable;

 public:
  std::shared_ptr<::hermes::vm::Runtime> rt;

  explicit HermesABIRuntimeImpl(const hermes::vm::RuntimeConfig &runtimeConfig)
      : HermesABIRuntime{&vtable},
        rt(hermes::vm::Runtime::create(runtimeConfig)) {}
};

/// Convenience function to cast the given HermesABIRuntime to a
/// HermesABIRuntimeImpl.
HermesABIRuntimeImpl *impl(HermesABIRuntime *abiRt) {
  return static_cast<HermesABIRuntimeImpl *>(abiRt);
}

HermesABIRuntime *make_hermes_runtime(const HermesABIRuntimeConfig *config) {
  return new HermesABIRuntimeImpl({});
}

void release_hermes_runtime(HermesABIRuntime *abiRt) {
  delete impl(abiRt);
}

constexpr HermesABIRuntimeVTable HermesABIRuntimeImpl::vtable = {
    release_hermes_runtime,
};

} // namespace

extern "C" {
const HermesABIVTable *get_hermes_abi_vtable() {
  static constexpr HermesABIVTable abiVtable = {
      make_hermes_runtime,
  };
  return &abiVtable;
}
} // extern "C"
