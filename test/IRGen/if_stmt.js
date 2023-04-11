/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function main(boop) {
  function foo() {
    if (boop) {  } else { }
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %main(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: object, globalObject: object, "main": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function main(boop: any): any
// CHECK-NEXT:frame = [boop: any, foo: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %boop: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [boop]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [foo]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:  %4 = StoreFrameInst %3: object, [foo]: any
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [boop@main]: any
// CHECK-NEXT:  %1 = CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
