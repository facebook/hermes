/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_PASSMANAGER_PASS_H
#define HERMES_OPTIMIZER_PASSMANAGER_PASS_H

#include "llvh/ADT/StringRef.h"

namespace hermes {

class Function;
class Module;
class Instruction;
class IRBuilder;

/// This class represents a pass, which is a transformation of the IR. Passes
/// are either Function passes, which transform one function, and are not
/// allowed to create new functions or modify other functions, or module
/// passes which operate on the entire module and are free to manipulate
/// multiple functions.
class Pass {
 public:
  enum class PassKind {
    Function,
    Module,
  };

 private:
  /// Stores the kind of derived class.
  const PassKind kind;
  /// The textual name of the pass.
  llvh::StringRef name;

 public:
  /// Constructor. \p K indicates the kind of pass this is.
  explicit Pass(Pass::PassKind K, llvh::StringRef name) : kind(K), name(name) {}

  virtual ~Pass() = default;

  /// Returns the kind of the pass.
  PassKind getKind() const {
    return kind;
  }

  /// Returns the textual name of the pass.
  llvh::StringRef getName() const {
    return name;
  }
};

class FunctionPass : public Pass {
 public:
  explicit FunctionPass(llvh::StringRef name)
      : Pass(Pass::PassKind::Function, name) {}
  ~FunctionPass() override = default;

  /// Runs the current pass on the function \p F.
  /// \returns true if the function was modified.
  virtual bool runOnFunction(Function *F) = 0;

  static bool classof(const Pass *S) {
    return S->getKind() == PassKind::Function;
  }
};

class ModulePass : public Pass {
 public:
  explicit ModulePass(llvh::StringRef name)
      : Pass(Pass::PassKind::Module, name) {}
  ~ModulePass() override = default;

  /// Runs the current pass on the module \p M.
  /// \returns true if module was modified.
  virtual bool runOnModule(Module *M) = 0;

  static bool classof(const Pass *S) {
    return S->getKind() == PassKind::Module;
  }
};

/// Pass header declaration.
#define PASS(ID, NAME, DESCRIPTION) Pass *create##ID();
#include "Passes.def"

} // namespace hermes

#endif // HERMES_OPTIMIZER_PASSMANAGER_PASS_H
