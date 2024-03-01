/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function fibonacci(n) {
   if (n){
     return n;
   } else {
     return fibonacci(n);
   }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "fibonacci": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %fibonacci(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "fibonacci": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function fibonacci(n: any): any
// CHECK-NEXT:frame = [n: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %fibonacci(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %n: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [n]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [n]: any
// CHECK-NEXT:       CondBranchInst %4: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [n]: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) globalObject: object, "fibonacci": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [n]: any
// CHECK-NEXT:  %10 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %9: any
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end
