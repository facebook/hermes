/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function empty () {
    switch (1) {}
}

function onlyDefault () {
    switch (1) {
        default: break;
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [empty, onlyDefault]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %empty()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "empty" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %onlyDefault()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "onlyDefault" : string
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %5 = StoreStackInst undefined : undefined, %4
// CHECK-NEXT:  %6 = LoadStackInst %4
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:function_end

// CHECK:function empty()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function onlyDefault()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = BranchInst %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %3 = BranchInst %BB2
// CHECK-NEXT:function_end
