/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -target=HBC -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// Perform stack promotion after inlining

function f1(num) {
    'use strict';
    function bar() {
        return num;
    }
    return bar();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "f1": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %f1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "f1": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f1(num: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %num: any
// CHECK-NEXT:       ReturnInst %0: any
// CHECK-NEXT:function_end
