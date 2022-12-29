/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_CACHENEWOBJECT_H
#define HERMES_OPTIMIZER_SCALAR_CACHENEWOBJECT_H

#include "hermes/IR/IR.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Inserts the CacheNewObjectInst if possible, to reduce the time spent
/// creating and populating a new hidden class.
class CacheNewObject : public FunctionPass {
 public:
  explicit CacheNewObject() : hermes::FunctionPass("CacheNewObject") {}
  ~CacheNewObject() override = default;

  bool runOnFunction(Function *F) override;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_DCE_H
