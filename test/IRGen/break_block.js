/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

var x = 0;
end: {
  x = 0;
  break end;
  x = 1;
}
x = 2;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "x": string
// CHECK-NEXT:  %2 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %2: any
// CHECK-NEXT:       StorePropertyLooseInst 0: number, globalObject: object, "x": string
// CHECK-NEXT:       StorePropertyLooseInst 0: number, globalObject: object, "x": string
// CHECK-NEXT:       StoreStackInst 0: number, %2: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       StorePropertyLooseInst 2: number, globalObject: object, "x": string
// CHECK-NEXT:       StoreStackInst 2: number, %2: any
// CHECK-NEXT:  %10 = LoadStackInst (:any) %2: any
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end
