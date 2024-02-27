/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

// Ensure that the "break" in the finally handler exits the correct
// loop.

function foo() {
    outer:
    for (;;) {
        try {
            for (;;) {
                bar2();
                break outer;
            }
            bar3();
        } finally {
            finally1();
            break;
        }
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function foo(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %6 = CatchInst (:any)
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "finally1": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst (:any) globalObject: object, "bar2": string
// CHECK-NEXT:  %12 = CallInst (:any) %11: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %15 = TryLoadGlobalPropertyInst (:any) globalObject: object, "finally1": string
// CHECK-NEXT:  %16 = CallInst (:any) %15: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end
