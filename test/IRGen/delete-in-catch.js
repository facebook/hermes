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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "result": string
// CHECK-NEXT:  %2 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %2: any
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:       StoreFrameInst %0: environment, %5: any, [e]: any
// CHECK-NEXT:       StorePropertyLooseInst false: boolean, globalObject: object, "result": string
// CHECK-NEXT:       StoreStackInst false: boolean, %2: any
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadStackInst (:any) %2: any
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %13 = CallInst (:any) %12: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        StoreStackInst %13: any, %2: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end
