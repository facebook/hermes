/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -target=HBC -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo1(a) {
    'use strict';
    var add = function() {
        return 100;
    }
    return add(a, 10);
}

function foo2(a) {
    'use strict';
    var add = function(a, b) {
        return a + b;
    }
    return add(a, 10);
}

function foo3(a) {
    'use strict';
    var add = function(a, b) {
        return a ? a : b;
    }
    return add(a, 10);
}

function foo4(a) {
    'use strict';
    var add = function(a, b) {
        if (a < 0)
            return -1;
        if (a == 0)
            return b;
        return a;
    }
    return add(a, 10);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo1": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo2": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo3": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo4": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %foo1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "foo1": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %foo2(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "foo2": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %foo3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "foo3": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %foo4(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "foo4": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo1(a: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 100: number
// CHECK-NEXT:function_end

// CHECK:function foo2(a: any): string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = BinaryAddInst (:string|number) %0: any, 10: number
// CHECK-NEXT:       ReturnInst %1: string|number
// CHECK-NEXT:function_end

// CHECK:function foo3(a: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       CondBranchInst %0: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = PhiInst (:any) %0: any, %BB2, 10: number, %BB0
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function foo4(a: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = BinaryLessThanInst (:boolean) %0: any, 0: number
// CHECK-NEXT:       CondBranchInst %1: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = PhiInst (:any) 10: number, %BB3, %0: any, %BB2, -1: number, %BB0
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BinaryEqualInst (:boolean) %0: any, 0: number
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB3, %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:function_end
