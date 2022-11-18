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

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [thisUndefined]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %thisUndefined() : object
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "thisUndefined" : string
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function thisUndefined() : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst globalObject : object
// CHECK-NEXT:function_end
