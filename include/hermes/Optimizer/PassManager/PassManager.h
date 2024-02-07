/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_PASSMANAGER_PASSMANAGER_H
#define HERMES_OPTIMIZER_PASSMANAGER_PASSMANAGER_H

#include "hermes/Optimizer/PassManager/Pass.h"

#include "llvh/ADT/StringRef.h"

#include <memory>
#include <vector>

namespace hermes {

/// The pass manager is responsible for running the transformation passes on the
/// whole module and on the functions in the module. The pass manager determines
/// the order of the passes, the order of the functions to be processed and the
/// invalidation of analysis.
class PassManager {
  std::vector<std::unique_ptr<Pass>> pipeline_;

 public:
  ~PassManager();

/// Add a pass by appending its name.
#define PASS(ID, NAME, DESCRIPTION) \
  void add##ID() {                  \
    addPass(hermes::create##ID());  \
  }
#include "Passes.def"

  /// Add a pass by name.
  bool addPassForName(llvh::StringRef name) {
#define PASS(ID, NAME, DESCRIPTION) \
  if (name == NAME) {               \
    add##ID();                      \
    return true;                    \
  }
#include "Passes.def"
    return false;
  }

  static std::string getCustomPassText() {
    return
#define PASS(ID, NAME, DESCRIPTION) NAME ": " DESCRIPTION "\n"
#include "Passes.def"
        ;
  }

  /// Add a pass by reference.
  void addPass(Pass *P);

  void run(Function *F);

  /// Run all the passes added.
  /// If IR verification is enabled:
  ///  Verify the IR between every pass. On the first failure, stop running any
  ///  more passes and \return false. If all passes verified correctly, \return
  ///  true.
  /// If IR verification is not enabled, \return true.
  bool run(Module *M);
};
} // namespace hermes
#undef DEBUG_TYPE
#endif // HERMES_OPTIMIZER_PASSMANAGER_PASSMANAGER_H
