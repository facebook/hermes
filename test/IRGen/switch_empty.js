/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function empty () {
    switch (1) {}
}

function onlyDefault () {
    switch (1) {
        default: break;
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "empty": string
// CHECK-NEXT:       DeclareGlobalVarInst "onlyDefault": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %empty(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "empty": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %onlyDefault(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "onlyDefault": string
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %6: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %6: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function empty(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function onlyDefault(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:function_end
