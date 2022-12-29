/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/ConstructorNewObjectCacheEntry.h"

#include "hermes/VM/HiddenClass.h"
#include "hermes/VM/SlotAcceptor.h"

namespace hermes {
namespace vm {

ConstructorNewObjectCacheEntry::ConstructorNewObjectCacheEntry(
    Runtime &runtime,
    llvh::ArrayRef<HiddenClass *> protoClazzes)
    : numProtoClazzes_{protoClazzes.size()} {
  auto *protos = getProtoClazzesPtr();
  for (auto *protoClazz : protoClazzes)
    new (protos++) WeakRoot<HiddenClass>(protoClazz, runtime);
}

void ConstructorNewObjectCacheEntry::markCachedHiddenClasses(
    WeakRootAcceptor &acceptor) {
  acceptor.acceptWeak(clazz_);
  auto *protos = getProtoClazzesPtr();
  for (size_t i = 0, e = numProtoClazzes_; i < e; ++i)
    acceptor.acceptWeak(protos[i]);
}

} // namespace vm
} // namespace hermes
