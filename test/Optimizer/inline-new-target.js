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
        // new.target is an inlining barrier.
        return new.target;
    }
    return f1();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "outer" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %outer()
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "outer" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function outer(a, b)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %f1()
// CHECK-NEXT:  %1 = CallInst %0 : closure, %f1(), empty, undefined : undefined
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:function_end

// CHECK:function f1()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst %new.target
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:function_end
