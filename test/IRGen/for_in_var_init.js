/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -dump-ir %s -O

function foo(obj) {
  for (var x = 10 in obj) {
      print(x);
  }
  return x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(obj: any): any
// CHECK-NEXT:frame = [obj: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [obj]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %5 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %6 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %7 = StoreFrameInst 10: number, [x]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:  %9 = StoreStackInst %8: any, %4: any
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:  %11 = GetPNamesInst %3: any, %4: any, %5: number, %6: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %13 = ReturnInst %12: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = GetNextPNameInst %10: any, %4: any, %5: number, %6: number, %3: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %15 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %16 = StoreFrameInst %15: any, [x]: any
// CHECK-NEXT:  %17 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %19 = CallInst (:any) %17: any, empty: any, empty: any, undefined: undefined, %18: any
// CHECK-NEXT:  %20 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %21 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
