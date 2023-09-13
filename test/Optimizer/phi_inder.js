/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

// Make sure we can remove all trampolines from our code.

function sink() {}

function recursive_phi(x) {
  var k = 1;
  var j = "hi";
  var t;

  for (var i = 0; i < 10; i++) {
    if (x > 3) {
      t = k;
      k = j;
      j = t;
    }
  }

  return k + j;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "sink": string
// CHECK-NEXT:       DeclareGlobalVarInst "recursive_phi": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %sink(): undefined
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "sink": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %recursive_phi(): string|number
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "recursive_phi": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function sink(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function recursive_phi(x: any): string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = PhiInst (:string|number) 1: number, %BB0, %9: string|number, %BB2
// CHECK-NEXT:  %3 = PhiInst (:string|number) "hi": string, %BB0, %10: string|number, %BB2
// CHECK-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %11: number, %BB2
// CHECK-NEXT:  %5 = BinaryGreaterThanInst (:boolean) %0: any, 3: number
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB3, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = BinaryAddInst (:string|number) %9: string|number, %10: string|number
// CHECK-NEXT:       ReturnInst %7: string|number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = PhiInst (:string|number) %3: string|number, %BB3, %2: string|number, %BB1
// CHECK-NEXT:  %10 = PhiInst (:string|number) %2: string|number, %BB3, %3: string|number, %BB1
// CHECK-NEXT:  %11 = FAddInst (:number) %4: number, 1: number
// CHECK-NEXT:  %12 = FLessThanInst (:boolean) %11: number, 10: number
// CHECK-NEXT:        CondBranchInst %12: boolean, %BB1, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end
