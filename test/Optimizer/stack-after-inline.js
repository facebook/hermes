/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -target=HBC -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// Perform stack promotion after inlining

function f1(num) {
    function bar() {
        return num;
    }
    return bar();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [f1]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %f1()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "f1" : string
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f1(num)
// CHECK-NEXT:frame = [num]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst %num
// CHECK-NEXT:function_end
