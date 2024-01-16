/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

(function () {
  // res should be number.
  var res = 0;

  function foo() {
    bar(123);
  }

  function bar(o) {
    res += o;
  }

  return foo;
});

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %""(): functionCode
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function ""(): object
// CHECK-NEXT:frame = [res: number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %foo(): functionCode
// CHECK-NEXT:       StoreFrameInst 0: number, [res]: number
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function foo(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:number) [res@""]: number
// CHECK-NEXT:  %1 = FAddInst (:number) %0: number, 123: number
// CHECK-NEXT:       StoreFrameInst %1: number, [res@""]: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
