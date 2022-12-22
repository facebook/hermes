/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_STACKPROMOTION_H
#define HERMES_OPTIMIZER_SCALAR_STACKPROMOTION_H

#include "hermes/IR/IR.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Promotes variables into stack allocations.
/// Removes all trivially unreachable functions.
///
/// FIXME: The pass as it is currently written has some more logic.
/// It attempts to avoid writing back to the frame in blocks which do not
/// themselves capture the variable.
/// This can result in unnecessary AllocStackInst at the top of the function.
/// It also emits unnecessary StoreStackInst to mitigate this problem,
/// due to potentially adding inconsistencies in where vars are initialized.
///
/// *This needs to be fixed* but due to not understanding the pass,
/// we can't fix it in a way that would make it able to be used in the Pipeline.
///
/// An example:
/// \code
///   function foo(x) {
///     if (g) {
///       ++x;
///       0 + function() { return x; }
///     }
///   }
///
/// results in the following IR after the second StackPromotion
/// (view with -Xdump-between-passes):
///
/// function foo(x) : undefined
/// frame = [x]
/// %BB0:
///   %0 = AllocStackInst $x
///   %1 = StoreStackInst undefined : undefined, %0
///   %2 = AllocStackInst $x
///   %3 = LoadParamInst %x
///   %4 = TryLoadGlobalPropertyInst globalObject : object, "g" : string
///   %5 = CondBranchInst %4, %BB1, %BB2
/// %BB1:
///   %6 = LoadStackInst %0
///   %7 = StoreFrameInst %6, [x]
///   %8 = StoreFrameInst %3, [x]
///   %9 = UnaryOperatorInst '++', %3
///   %10 = StoreFrameInst %9 : number|bigint, [x]
///   %11 = CreateFunctionInst %""()
///   %12 = BinaryOperatorInst '+', 0 : number, %11 : closure
///   %13 = BranchInst %BB3
/// %BB3:
///   %14 = ReturnInst undefined : undefined
/// %BB2:
///   %15 = StoreStackInst %3, %0
///   %16 = BranchInst %BB4
/// %BB4:
///   %17 = LoadStackInst %0
///   %18 = StoreFrameInst %17, [x]
///   %19 = BranchInst %BB3
/// function_end
///
/// There are two AllocStackInsts for $x.
/// %2 is completely unused, while %0 is only initialized due to %1 which
/// was added specially by this pass.
/// Because %x is still in the frame, subsequent StackPromotion passes will
/// promote it again, resulting in more AllocStackInsts.
class StackPromotion : public ModulePass {
 public:
  explicit StackPromotion() : ModulePass("StackPromotion") {}
  ~StackPromotion() override = default;

  bool runOnModule(Module *M) override;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_STACKPROMOTION_H
