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
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %""(): functionCode
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [res: number]

// CHECK:function ""(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, 0: number, [%VS0.res]: number
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:function foo(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS0.res]: number
// CHECK-NEXT:  %2 = FAddInst (:number) %1: number, 123: number
// CHECK-NEXT:       StoreFrameInst %0: environment, %2: number, [%VS0.res]: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
