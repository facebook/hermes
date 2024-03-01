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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function foo(obj: any): any
// CHECK-NEXT:frame = [obj: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [obj]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %7 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %8 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 10: number, [x]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:        StoreStackInst %10: any, %6: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %5: any, %6: any, %7: number, %8: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %12: any, %6: any, %7: number, %8: number, %5: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %17 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %17: any, [x]: any
// CHECK-NEXT:  %19 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %21 = CallInst (:any) %19: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %20: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end
