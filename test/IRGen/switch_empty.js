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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [empty, onlyDefault]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %empty#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "empty" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %onlyDefault#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "onlyDefault" : string
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %6 = StoreStackInst undefined : undefined, %5
// CHECK-NEXT:  %7 = LoadStackInst %5
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function empty#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{empty#0#1()#2}
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function onlyDefault#0#1()#3
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{onlyDefault#0#1()#3}
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = BranchInst %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = BranchInst %BB2
// CHECK-NEXT:function_end
