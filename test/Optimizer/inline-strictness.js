/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function outer(a, b) {
    function f1() {
        'use strict';
        return f2() + 1;
    }
    function f2() {
        return a;
    }
    return f1();
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

// CHECK:function outer(a: any, b: any): string|number
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %outer(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [a]: any
// CHECK-NEXT:  %4 = BinaryAddInst (:string|number) %2: any, 1: number
// CHECK-NEXT:       ReturnInst %4: string|number
// CHECK-NEXT:function_end
