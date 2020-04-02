/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_BUNDLER_UTILS_H
#define HERMES_OPTIMIZER_SCALAR_BUNDLER_UTILS_H

#include <llvm/ADT/DenseSet.h>
#include "hermes/IR/IR.h"
#include "llvm/ADT/SetVector.h"

namespace hermes {
/// This class contains code that is specific to the bundler.
class BundlerUtils {
  typedef unsigned int ModuleID;

  /// Map of a JSmodule number to an anonymous fun representing the JSmodule.
  llvm::DenseMap<ModuleID, Function *> JSmoduleTable_;
  llvm::DenseMap<Function *, ModuleID> revJSmoduleTable_;

  /// Map of the set of other JSmodules that a JSmodule imports.
  llvm::DenseMap<ModuleID, llvm::DenseSet<ModuleID>> importsMap_;
  llvm::DenseMap<ModuleID, llvm::DenseSet<ModuleID>> revImportsMap_;

  /// Set of JSmodules that we can process with the unique JSmodule that
  /// requires it.  This expands the scope of the set constraint analysis.
  llvm::SetVector<ModuleID> inlineableJSmodules_;

  /// A bundler-specific function to figure out 'require' dependencies between
  /// functions defined at the top level. These functions (called JSModules)
  /// appear as:
  /// __d(N, function (...) {  },
  /// and they require other JSmodules using require(N).
  void createJSModuleDependencies(Module *M);

  /// An algorithm to mark some of the JSmodules as 'inlineable',
  /// implying that they should be analyzed together with the
  /// JSmodule importing them.
  void identifyInlineableJSModules();

  /// A define looks like this:
  /// __d(function (require, module, ..) { /* code for JSmodule */ }, N);
  /// where __d is a function implemented by the framework, the first arg
  /// is the JSModule and the second arg is a numeric, globally unique ID for
  /// it.
  bool isJSModuleDefine(Instruction *, ModuleID &, Function *&);

  /// Finds the requires statements inside the function F.
  void findFunctionRequires(Function *F, llvm::DenseSet<ModuleID> &requires);

  llvm::DenseMap<Function *, llvm::SetVector<Function *>> nestedMap_;

  /// Which kind of bundling are we handling?
  BundlerKind xmform_;

  /// Helper functions to deal with the bundling form
  bool isJSModuleRequiresMetroMin(Instruction *, unsigned int &);
  bool isJSModuleExportsMetroMin(Instruction *, Value *&);

 public:
  void initialize(
      Module *M,
      llvm::DenseMap<Function *, llvm::SetVector<Function *>> &nm,
      BundlerKind xmform) {
    nestedMap_ = nm;
    xmform_ = xmform;
    if (xmform_ != hermes::BundlerKind::metromin)
      llvm_unreachable("Currently only metro bundling is supported!\n");
    createJSModuleDependencies(M);
    identifyInlineableJSModules();
  }

  ~BundlerUtils() {
    // Note: nestedMap_ is cleaned up by closure analysis.
    importsMap_.clear();
    revImportsMap_.clear();
    JSmoduleTable_.clear();
    revJSmoduleTable_.clear();
  }

  /// Is the function F a JSModule, if so, set modId to the module ID.
  bool isJSModule(Function *F, ModuleID &modId) {
    auto rmt_it = revJSmoduleTable_.find(F);
    if (rmt_it != revJSmoduleTable_.end()) {
      modId = rmt_it->second;
      return true;
    }
    return false;
  }

  /// Is the function F a JSModule
  bool isJSModule(Function *F) {
    unsigned int m;
    return isJSModule(F, m);
  }

  /// Is the module ID (being a JSModule) also an inlineable one?
  bool isInlineableJSModule(ModuleID modId) {
    return inlineableJSmodules_.count(modId) != 0;
  }

  /// Retrieve the Function corresponding to a module ID
  Function *modId2Function(ModuleID modId) {
    auto mt_it = JSmoduleTable_.find(modId);
    if (mt_it == JSmoduleTable_.end()) {
      llvm_unreachable("Expected to find id in JSmoduleTable\n");
    }
    Function *d = mt_it->second;
    return d;
  }

  /// If a module modId has imports, return the set in imports and return true.
  /// Else return false;
  bool getImportsOfModule(ModuleID modId, llvm::DenseSet<ModuleID> &imports) {
    auto im_it = importsMap_.find(modId);
    if (im_it != importsMap_.end()) {
      imports = im_it->second;
      return true;
    }
    return false;
  }

  /// Is the given instruction a store into 'module.exports', where
  /// module comes from parameter list, and exports is the property
  /// name? If so, return the value being in Value.
  bool isJSModuleExports(Instruction *, Value *&);

  /// Looks for a call require(num), where 'require' is:
  /// 1. a parameter of the anonymous function representing a module, or
  /// 2. loaded from the frame of the anonymous function.
  bool isJSModuleRequires(Instruction *, ModuleID &);
};
} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_BUNDLER_UTILS_H
