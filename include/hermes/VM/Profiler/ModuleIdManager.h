#ifndef HERMES_VM_PROFILER_MODULEIDMANAGER_H
#define HERMES_VM_PROFILER_MODULEIDMANAGER_H

#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/VM/RuntimeModule.h"

#include "llvm/ADT/DenseMap.h"

#include <vector>

namespace hermes {
namespace vm {

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
  /// In charge of ref count for RuntimeModule.
  ModuleIdMap moduleIds_;

  /// Maps from module id to RuntimeModule.
  /// Does not participate in RuntimeModule ref count.
  llvm::DenseMap<ModuleId, RuntimeModule *> idToModuleMap_;

 public:
  /// Find existing \p module or add it if does not exist.
  /// \return the module's id.
  ModuleIdManager::ModuleId findOrAdd(RuntimeModule *module);

  /// Find runtime module from its id.
  OptValue<RuntimeModule *> getModule(ModuleId moduleId) const;

  /// \return the id => BCProvider map.
  ModuleIdManager::BCProviderMap generateBCProviderMap() const;

  /// Release all modules.
  void clear();
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PROFILER_MODULEIDMANAGER_H
