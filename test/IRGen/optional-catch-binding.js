/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo(f, g) {
  try {
    return f();
  } catch {
    return g();
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %foo()
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo(f, g)
// CHECK-NEXT:frame = [f, g]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %f
// CHECK-NEXT:  %1 = StoreFrameInst %0, [f]
// CHECK-NEXT:  %2 = LoadParamInst %g
// CHECK-NEXT:  %3 = StoreFrameInst %2, [g]
// CHECK-NEXT:  %4 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst
// CHECK-NEXT:  %6 = LoadFrameInst [g]
// CHECK-NEXT:  %7 = CallInst %6, undefined : undefined
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst [f]
// CHECK-NEXT:  %11 = CallInst %10, undefined : undefined
// CHECK-NEXT:  %12 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = TryEndInst
// CHECK-NEXT:  %14 = ReturnInst %11
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %15 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %16 = TryEndInst
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %18 = BranchInst %BB3
// CHECK-NEXT:function_end
