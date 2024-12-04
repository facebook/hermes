/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "cachenewobject"

#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

Pass *createCacheNewObject() {
  /// Inserts the CacheNewObjectInst if possible, to reduce the time spent
  /// creating and populating a new hidden class.
  class ThisPass : public FunctionPass {
   public:
    explicit ThisPass() : hermes::FunctionPass("CacheNewObject") {}
    ~ThisPass() override = default;

    bool runOnFunction(Function *F) override {
      return false;
    }
  };

  return new ThisPass();
}

} // namespace hermes

#undef DEBUG_TYPE
