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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "fibonacci": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %fibonacci(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: object, globalObject: object, "fibonacci": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function fibonacci(n: any): any
// CHECK-NEXT:frame = [n: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %n: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [n]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [n]: any
// CHECK-NEXT:  %3 = CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [n]: any
// CHECK-NEXT:  %5 = ReturnInst %4: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) globalObject: object, "fibonacci": string
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [n]: any
// CHECK-NEXT:  %8 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %7: any
// CHECK-NEXT:  %9 = ReturnInst %8: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = UnreachableInst
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = BranchInst %BB3
// CHECK-NEXT:function_end
