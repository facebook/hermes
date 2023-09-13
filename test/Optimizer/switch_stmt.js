/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen %s

function backwards_branch() {
  for (var i = 0; i < 4; i++) {
    switch (i) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
        break;
    }
    i = 2;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "backwards_branch": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %backwards_branch(): undefined
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "backwards_branch": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function backwards_branch(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = PhiInst (:number) 0: number, %BB0, 3: number, %BB1
// CHECK-NEXT:       SwitchInst %1: number, %BB1, 0: number, %BB1, 1: number, %BB1, 2: number, %BB1, 3: number, %BB1, 4: number, %BB1, 5: number, %BB1, 6: number, %BB1, 7: number, %BB1, 8: number, %BB1, 9: number, %BB1
// CHECK-NEXT:function_end
