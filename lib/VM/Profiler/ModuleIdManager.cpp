/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/Profiler/ModuleIdManager.h"

#include "hermes/VM/Domain.h"
#include "hermes/VM/RuntimeModule-inline.h"

#include <algorithm>

namespace hermes {
namespace vm {

ModuleIdManager::ModuleId ModuleIdManager::findOrAdd(RuntimeModule *module) {
  auto iter = moduleIds_.find(module);
  if (iter != moduleIds_.end()) {
    // Already existed.
    return iter->second;
  }
  uint32_t moduleId = moduleIds_.size();
  moduleIds_[module] = moduleId;
  idToModuleMap_[moduleId] = module;
  if (domains_.empty() || domains_.back() != module->getDomainUnsafe()) {
    // Only add the next domain if it wasn't the last one added,
    // to reduce a possibly large domains_ list.
    domains_.push_back(module->getDomainUnsafe());
  }
  return moduleId;
}

OptValue<RuntimeModule *> ModuleIdManager::getModule(ModuleId moduleId) const {
  auto iter = idToModuleMap_.find(moduleId);
  if (iter == idToModuleMap_.end()) {
    return llvm::None;
  }
  return iter->second;
}

ModuleIdManager::BCProviderMap ModuleIdManager::generateBCProviderMap() const {
  /// Maps from module id to its BCProvider.
  BCProviderMap bcProviderMap;
  for (const auto &entry : idToModuleMap_) {
    assert(bcProviderMap.count(entry.first) == 0 && "Duplicate module ID");
    bcProviderMap[entry.first] = entry.second->getBytecodeSharedPtr();
  }
  return bcProviderMap;
}

void ModuleIdManager::markRoots(SlotAcceptorWithNames &acceptor) {
  for (Domain *&domain : domains_) {
    acceptor.acceptPtr(domain);
  }
}

void ModuleIdManager::clear() {
  // Release module references.
  moduleIds_.clear();
  // Release domain references.
  domains_.clear();
}

} // namespace vm
} // namespace hermes
