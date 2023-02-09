/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function outer(a, b) {
    'use strict'
    function f1() {
        return f2() + 1;
    }
    function f2() {
        return a;
    }
    return f1();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "outer" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %outer() : string|number
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "outer" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function outer(a, b) : string|number
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = CreateFunctionInst %f2()
// CHECK-NEXT:  %3 = BinaryOperatorInst '+', %0, 1 : number
// CHECK-NEXT:  %4 = ReturnInst %3 : string|number
// CHECK-NEXT:function_end

// CHECK:function f2() [allCallsitesKnown]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [a@outer]
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:function_end
