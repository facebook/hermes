/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc %s -dump-ir | %FileCheckOrRegen --match-full-lines %s

// Check that we don't replace GetConstructedObjectInst with a
// value which does not also have an object type.
//
// IRVerifier error: StoreFrameInst: Value type mismatch in function

function main() {
  var x = null;
  function y() {}
  return function value() {
    return x || (x = new y());
  };
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %main(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "main": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [x: null|object, y: object]

// CHECK:function main(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS1: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS1: any, %y(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: object, [%VS1.y]: object
// CHECK-NEXT:       StoreFrameInst %0: environment, null: null, [%VS1.x]: null|object
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %VS1: any, %value(): functionCode
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function y(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function value(): null|object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:null|object) %0: environment, [%VS1.x]: null|object
// CHECK-NEXT:       CondBranchInst %1: null|object, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = LoadFrameInst (:object) %0: environment, [%VS1.y]: object
// CHECK-NEXT:  %4 = CreateThisInst (:object) %3: object, empty: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %4: object, [%VS1.x]: null|object
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = PhiInst (:null|object) %1: null|object, %BB0, %4: object, %BB1
// CHECK-NEXT:       ReturnInst %7: null|object
// CHECK-NEXT:function_end
