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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "empty" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "onlyDefault" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %empty()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "empty" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %onlyDefault()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "onlyDefault" : string
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %7 = StoreStackInst undefined : undefined, %6
// CHECK-NEXT:  %8 = LoadStackInst %6
// CHECK-NEXT:  %9 = ReturnInst %8
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
