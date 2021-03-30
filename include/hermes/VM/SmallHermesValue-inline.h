/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SMALLHERMESVALUE_INLINE_H
#define HERMES_VM_SMALLHERMESVALUE_INLINE_H

#include "hermes/VM/SmallHermesValue.h"

#include "hermes/VM/HermesValue-inline.h"

namespace hermes {
namespace vm {

void SmallHermesValueAdaptor::setInGC(SmallHermesValueAdaptor hv, GC *gc) {
  HermesValue::setInGC(hv, gc);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SMALLHERMESVALUE_INLINE_H
