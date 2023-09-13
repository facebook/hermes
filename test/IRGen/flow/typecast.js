/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir -O0 %s | %FileCheckOrRegen %s --match-full-lines

(function(){
  return (Math.PI : number) * 3.0;
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %""(): any
// CHECK-NEXT:  %3 = CallInst [njsf] (:any) %2: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreStackInst %3: any, %0: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %0: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function ""(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Math": string
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: any, "PI": string
// CHECK-NEXT:  %2 = CheckedTypeCastInst (:number) %1: any
// CHECK-NEXT:  %3 = BinaryMultiplyInst (:any) %2: number, 3: number
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end
