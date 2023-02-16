/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines

// Ensure that catch variables are not treated as global.
var result;
try {
    foo();
} catch(e) {
    result = delete e;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = [e: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "result": string
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %2 = StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst (:any)
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [e]: any
// CHECK-NEXT:  %6 = StorePropertyLooseInst false: boolean, globalObject: object, "result": string
// CHECK-NEXT:  %7 = StoreStackInst false: boolean, %1: any
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %10 = ReturnInst (:any) %9: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %12 = CallInst (:any) %11: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %13 = StoreStackInst %12: any, %1: any
// CHECK-NEXT:  %14 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = TryEndInst
// CHECK-NEXT:  %16 = BranchInst %BB3
// CHECK-NEXT:function_end
