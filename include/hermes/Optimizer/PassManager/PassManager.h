/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_PASSMANAGER_PASSMANAGER_H
#define HERMES_OPTIMIZER_PASSMANAGER_PASSMANAGER_H

#include "hermes/Optimizer/PassManager/Pass.h"

#include "hermes/AST/Context.h"
#include "llvh/ADT/StringRef.h"

#include <memory>
#include <utility>
#include <vector>

namespace hermes {

/// The pass manager is responsible for running the transformation passes on the
/// whole module and on the functions in the module. The pass manager determines
/// the order of the passes, the order of the functions to be processed and the
/// invalidation of analysis.
class PassManager {
  const CodeGenerationSettings &cgSettings_;
  std::vector<std::unique_ptr<Pass>> pipeline_;

 public:
  explicit PassManager(const CodeGenerationSettings &settings);

  ~PassManager();

/// Add a pass by appending its name.
#define PASS(ID, NAME, DESCRIPTION)                       \
  void add##ID() {                                        \
    addPass(std::unique_ptr<Pass>(hermes::create##ID())); \
  }
#include "Passes.def"

  /// Add a pass by name.
  /// Note: Only works for passes that are part of Passes.def.
  bool addPassForName(llvh::StringRef name) {
#define PASS(ID, NAME, DESCRIPTION) \
  if (name == NAME) {               \
    add##ID();                      \
    return true;                    \
  }
#include "Passes.def"
    return false;
  }

  /// Lists and describes the passes in Passes.def.
  static llvh::StringRef getCustomPassText() {
    return
#define PASS(ID, NAME, DESCRIPTION) NAME ": " DESCRIPTION "\n"
#include "Passes.def"
        ;
  }

  /// Adds the pass \p Pass with the provided \p args to this pass manager.
  template <typename Pass, typename... Args>
  void addPass(Args &&...args) {
    addPass(std::make_unique<Pass>(std::forward<Args>(args)...));
  }

  /// Runs this pass manager on the given Function \p F.
  /// \pre Only FunctionPasses are registered with this PassManager.
  void run(Function *F);

  /// Runs this pass manager on the given Module \p M.
  void run(Module *M);

 private:
  /// Adds \p P to the pipeline managed by this pass manager.
  void addPass(std::unique_ptr<Pass> P);

  /// \return A pass that dumps the IR before/after \p pass runs. The options
  /// controlling the IR dump live in CodeGenSettings.
  std::unique_ptr<Pass> makeDumpPass(std::unique_ptr<Pass> pass);
};
} // namespace hermes

#endif // HERMES_OPTIMIZER_PASSMANAGER_PASSMANAGER_H
