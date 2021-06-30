/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_RUNTIMEMODULE_INLINE_H
#define HERMES_VM_RUNTIMEMODULE_INLINE_H

#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

inline Handle<Domain> RuntimeModule::getDomain(Runtime *runtime) {
  auto optionalHandle = domain_.get(runtime_, &runtime_->getHeap());
  assert(optionalHandle && "RuntimeModule has an invalid Domain");
  return optionalHandle.getValue();
}

inline Domain *RuntimeModule::getDomainUnsafe(Runtime *runtime) {
  Domain *domain = getNoHandle(domain_, &runtime_->getHeap());
  assert(domain && "RuntimeModule has an invalid Domain");
  return domain;
}

inline Domain *RuntimeModule::getDomainForSamplingProfiler() {
  // Do not use a read barrier here, as this is called from the SamplingProfiler
  // signal handler. The signal handler may have interrupted another read/write
  // barrier, which the GC isn't prepared to handle. Don't use this anywhere
  // else.
  OptValue<Domain *> domain = domain_.unsafeGetOptionalNoReadBarrier();
  assert(domain && "RuntimeModule has an invalid Domain");
  return domain.getValue();
}

} // namespace vm
} // namespace hermes

#endif
