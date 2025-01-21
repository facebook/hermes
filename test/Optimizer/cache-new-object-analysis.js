/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -fno-inline -dump-ir %s -O | %FileCheckOrRegen --match-full-lines %s

function main() {

// Make sure allCallsitesKnownInStrictMode is true.
function simple(x, y) {
  this.x = x;
  this.y = y;
}

return new simple(1, 2);

}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %main(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "main": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %simple(): functionCode
// CHECK-NEXT:  %2 = CreateThisInst (:object) %1: object, empty: any
// CHECK-NEXT:  %3 = CallInst (:undefined) %1: object, %simple(): functionCode, true: boolean, empty: any, undefined: undefined, %2: object, 1: number, 2: number
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function simple(x: any, y: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       CacheNewObjectInst %1: object, %2: undefined|object, "x": string, "y": string
// CHECK-NEXT:  %4 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StorePropertyLooseInst %4: any, %1: object, "x": string
// CHECK-NEXT:       StorePropertyLooseInst %5: any, %1: object, "y": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
