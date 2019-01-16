#include "hermes/VM/Profiler/ModuleIdManager.h"

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
  // Add ref for runtime module.
  module->addUser();
  idToModuleMap_[moduleId] = module;
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

void ModuleIdManager::clear() {
  // Release module references.
  for (const auto &entry : moduleIds_) {
    entry.first->removeUser();
  }
  moduleIds_.clear();
}

} // namespace vm
} // namespace hermes
