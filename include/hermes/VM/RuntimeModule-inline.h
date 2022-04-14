/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_RUNTIMEMODULE_INLINE_H
#define HERMES_VM_RUNTIMEMODULE_INLINE_H

#include "hermes/VM/Runtime.h"
#include "hermes/VM/WeakRoot-inline.h"

namespace hermes {
namespace vm {

inline Handle<Domain> RuntimeModule::getDomain(Runtime &runtime) {
  return runtime.makeHandle(getDomainUnsafe(runtime));
}

inline Domain *RuntimeModule::getDomainUnsafe(Runtime &runtime) {
  Domain *domain = domain_.get(runtime_, runtime_.getHeap());
  assert(domain && "RuntimeModule has an invalid Domain");
  return domain;
}

inline Domain *RuntimeModule::getDomainForSamplingProfiler(PointerBase &base) {
  // Do not use a read barrier here, as this is called from the SamplingProfiler
  // signal handler. The signal handler may have interrupted another read/write
  // barrier, which the GC isn't prepared to handle. Don't use this anywhere
  // else.
  Domain *domain = domain_.getNoBarrierUnsafe(runtime_);
  assert(domain && "RuntimeModule has an invalid Domain");
  return domain;
}

} // namespace vm
} // namespace hermes

#endif
