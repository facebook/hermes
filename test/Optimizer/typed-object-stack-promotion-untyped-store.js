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
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %Symbol(): functionCode
// CHECK-NEXT:  %1 = AllocTypedNonEnumObjectInst (:object) null: null
// CHECK-NEXT:       StorePropertyStrictInst %1: object, %0: object, "prototype": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) empty: any, empty: any, %Object(): functionCode
// CHECK-NEXT:  %4 = AllocTypedNonEnumObjectInst (:object) null: null
// CHECK-NEXT:       StorePropertyStrictInst %4: object, %3: object, "prototype": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) empty: any, empty: any, %TestClass(): functionCode
// CHECK-NEXT:  %7 = AllocTypedNonEnumObjectInst (:object) null: null
// CHECK-NEXT:       StorePropertyStrictInst %7: object, %6: object, "prototype": string
// CHECK-NEXT:  %9 = AllocTypedObjectInst (:object) %7: object, "myName": string, "": string
// CHECK-NEXT:        PrStoreInst "Dracula ": string, %9: object, 0: number, "myName": string, false: boolean
// CHECK-NEXT:        StorePropertyStrictInst "Wolf Man": string, %9: object, "myName": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:base constructor Symbol(): any [typed,noReturn]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "TypeError": string
// CHECK-NEXT:  %1 = CreateThisInst (:undefined|object) %0: any, %0: any, empty: any
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, false: boolean, empty: any, %0: any, %1: undefined|object, "Symbol is not a constructor": string
// CHECK-NEXT:  %3 = GetConstructedObjectInst (:object) %1: undefined|object, %2: any
// CHECK-NEXT:       ThrowInst %3: object
// CHECK-NEXT:function_end

// CHECK:function Object(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:base constructor TestClass(name: string): undefined [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadParamInst (:string) %name: string
// CHECK-NEXT:       PrStoreInst %1: string, %0: object, 0: number, "myName": string, false: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
