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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %foo(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(f: any, g: any): any
// CHECK-NEXT:frame = [f: any, g: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %f: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [f]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %g: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [g]: any
// CHECK-NEXT:  %4 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [g]: any
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %8 = ReturnInst %7: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [f]: any
// CHECK-NEXT:  %11 = CallInst (:any) %10: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %12 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = TryEndInst
// CHECK-NEXT:  %14 = ReturnInst %11: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %15 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %16 = TryEndInst
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %18 = BranchInst %BB3
// CHECK-NEXT:function_end
