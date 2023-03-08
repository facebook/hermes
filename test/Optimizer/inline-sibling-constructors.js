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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %outer(): closure
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "outer": string
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function outer(): closure
// CHECK-NEXT:frame = [Point: closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %Point(): undefined
// CHECK-NEXT:  %1 = StoreFrameInst %0: closure, [Point]: closure
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %makePoint(): object
// CHECK-NEXT:  %3 = ReturnInst %2: closure
// CHECK-NEXT:function_end

// CHECK:function Point(x: any, y: any, z: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %this: object
// CHECK-NEXT:  %1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %3 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  %4 = StorePropertyStrictInst %1: any, %0: object, "x": string
// CHECK-NEXT:  %5 = StorePropertyStrictInst %2: any, %0: object, "y": string
// CHECK-NEXT:  %6 = StorePropertyStrictInst %3: any, %0: object, "z": string
// CHECK-NEXT:  %7 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function makePoint(x: any, y: any, z: any): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  %3 = LoadFrameInst (:closure) [Point@outer]: closure
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: closure, "prototype": string
// CHECK-NEXT:  %5 = CreateThisInst (:object) %4: any, %3: closure
// CHECK-NEXT:  %6 = ConstructInst (:undefined) %3: closure, %Point(): undefined, empty: any, %5: object, %0: any, %1: any, %2: any
// CHECK-NEXT:  %7 = ReturnInst %5: object
// CHECK-NEXT:function_end
