/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

class TestClass {
  myName: string;
  constructor(name: string) {
    this.myName = name;
  }
}
function writeTomyName(target, value) {
  target.myName = value;
}
const testObject = new TestClass("Dracula ");
writeTomyName(testObject, "Wolf Man");

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %TestClass(): functionCode
// CHECK-NEXT:  %1 = AllocTypedNonEnumObjectInst (:object) null: null
// CHECK-NEXT:       StorePropertyStrictInst %1: object, %0: object, "prototype": string
// CHECK-NEXT:  %3 = AllocTypedObjectInst (:object) %1: object, "myName": string, "": string
// CHECK-NEXT:       PrStoreInst "Dracula ": string, %3: object, 0: number, "myName": string, false: boolean
// CHECK-NEXT:       StorePropertyStrictInst "Wolf Man": string, %3: object, "myName": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:base constructor TestClass(name: string): undefined [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadParamInst (:string) %name: string
// CHECK-NEXT:       PrStoreInst %1: string, %0: object, 0: number, "myName": string, false: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
