/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ir -O %s | %FileCheckOrRegen --match-full-lines %s

function thisUndefined () {
    function inner() {
        return this;
    }
    return inner();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [thisUndefined]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %thisUndefined#0#1()#2 : object, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "thisUndefined" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function thisUndefined#0#1()#2 : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{thisUndefined#0#1()#2}
// CHECK-NEXT:  %1 = ReturnInst globalObject : object
// CHECK-NEXT:function_end
