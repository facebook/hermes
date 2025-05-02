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
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %outer(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "outer": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [Point: object]

// CHECK:function outer(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %Point(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: object, [%VS0.Point]: object
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %makePoint(): functionCode
// CHECK-NEXT:       ReturnInst %3: object
// CHECK-NEXT:function_end

// CHECK:function Point(x: any, y: any, z: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       CacheNewObjectInst %0: any, %1: undefined|object, "x": string, "y": string, "z": string
// CHECK-NEXT:  %3 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %z: any
// CHECK-NEXT:       StorePropertyStrictInst %3: any, %0: any, "x": string
// CHECK-NEXT:       StorePropertyStrictInst %4: any, %0: any, "y": string
// CHECK-NEXT:       StorePropertyStrictInst %5: any, %0: any, "z": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function makePoint(x: any, y: any, z: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %3 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  %4 = LoadFrameInst (:object) %0: environment, [%VS0.Point]: object
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: object, "prototype": string
// CHECK-NEXT:  %6 = AllocObjectLiteralInst (:object) %5: any
// CHECK-NEXT:       CacheNewObjectInst %6: object, %4: object, "x": string, "y": string, "z": string
// CHECK-NEXT:       StorePropertyStrictInst %1: any, %6: object, "x": string
// CHECK-NEXT:       StorePropertyStrictInst %2: any, %6: object, "y": string
// CHECK-NEXT:        StorePropertyStrictInst %3: any, %6: object, "z": string
// CHECK-NEXT:        ReturnInst %6: object
// CHECK-NEXT:function_end
