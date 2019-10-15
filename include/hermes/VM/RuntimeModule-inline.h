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

inline Handle<Domain> RuntimeModule::getDomain(Runtime *) {
  assert(domain_.isValid() && "RuntimeModule has an invalid Domain");
  return domain_.get(runtime_);
}

inline Domain *RuntimeModule::getDomainUnsafe() {
  assert(domain_.isValid() && "RuntimeModule has an invalid Domain");
  return vmcast<Domain>(domain_.unsafeGetHermesValue());
}

} // namespace vm
} // namespace hermes

#endif
