/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function outer() {
    'use strict'
    function Point(x, y, z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }
    function makePoint(x, y, z) {
        return new Point(x, y, z);
    }
    return makePoint;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %outer(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "outer": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function outer(): object
// CHECK-NEXT:frame = [Point: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %outer(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %Point(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [Point]: object
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %makePoint(): functionCode
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function Point(x: any, y: any, z: any): undefined [allCallsitesKnownInStrictMode,unreachable]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function makePoint(x: any, y: any, z: any): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %outer(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %3 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  %4 = LoadFrameInst (:object) %0: environment, [Point@outer]: object
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: object, "prototype": string
// CHECK-NEXT:  %6 = CreateThisInst (:object) %5: any, %4: object
// CHECK-NEXT:       StorePropertyStrictInst %1: any, %6: object, "x": string
// CHECK-NEXT:       StorePropertyStrictInst %2: any, %6: object, "y": string
// CHECK-NEXT:       StorePropertyStrictInst %3: any, %6: object, "z": string
// CHECK-NEXT:        ReturnInst %6: object
// CHECK-NEXT:function_end
