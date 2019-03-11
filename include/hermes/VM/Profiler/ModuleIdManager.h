/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_PROFILER_MODULEIDMANAGER_H
#define HERMES_VM_PROFILER_MODULEIDMANAGER_H

#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/VM/RuntimeModule.h"

#include "llvm/ADT/DenseMap.h"

#include <vector>

namespace hermes {
namespace vm {

class Domain;

/// Manages profiler module/id/BCProvider mapping.
class ModuleIdManager {
 public:
  using ModuleId = uint32_t;
  /// Module id => BCProvider.
  using BCProviderMap =
      llvm::DenseMap<ModuleId, std::shared_ptr<hbc::BCProvider>>;

 private:
  /// RuntimeModule => Module id.
  using ModuleIdMap = llvm::DenseMap<RuntimeModule *, ModuleId>;

  /// Maps from RuntimeModule to its id.
  ModuleIdMap moduleIds_;

  /// Maps from module id to RuntimeModule.
  llvm::DenseMap<ModuleId, RuntimeModule *> idToModuleMap_;

  /// Domains to be kept alive by this ModuleIdManager.
  llvm::SmallVector<Domain *, 1> domains_;

 public:
  /// Find existing \p module or add it if does not exist.
  /// \return the module's id.
  ModuleIdManager::ModuleId findOrAdd(RuntimeModule *module);

  /// Find runtime module from its id.
  OptValue<RuntimeModule *> getModule(ModuleId moduleId) const;

  /// \return the id => BCProvider map.
  ModuleIdManager::BCProviderMap generateBCProviderMap() const;

  /// Mark roots that are kept alive by the ModuleIdManager.
  /// Marks the domains of all registered RuntimeModules.
  void markRoots(SlotAcceptorWithNames &acceptor);

  /// Release all modules.
  void clear();
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PROFILER_MODULEIDMANAGER_H
