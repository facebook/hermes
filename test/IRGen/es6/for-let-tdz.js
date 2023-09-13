/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xenable-tdz -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -Xenable-tdz -custom-opt=simplestackpromotion -dump-ir %s > /dev/null

// Verify code generation for a scoped for loop

function check_for_let() {
    for(let i = 0; i < 10; ++i)
        print(i);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "check_for_let": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %check_for_let(): any
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "check_for_let": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function check_for_let(): any
// CHECK-NEXT:frame = [i: any|empty, i#1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst empty: empty, [i]: any|empty
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any|empty
// CHECK-NEXT:  %2 = AllocStackInst (:boolean) $?anon_0_first: any
// CHECK-NEXT:       StoreStackInst true: boolean, %2: boolean
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst (:any|empty) [i]: any|empty
// CHECK-NEXT:  %6 = UnionNarrowTrustedInst (:any) %5: any|empty
// CHECK-NEXT:       StoreFrameInst %6: any, [i#1]: any
// CHECK-NEXT:  %8 = LoadStackInst (:boolean) %2: boolean
// CHECK-NEXT:       CondBranchInst %8: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:  %11 = BinaryLessThanInst (:any) %10: any, 10: number
// CHECK-NEXT:        CondBranchInst %11: any, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:  %14 = UnaryIncInst (:any) %13: any
// CHECK-NEXT:        StoreFrameInst %14: any, [i#1]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %17 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:  %19 = CallInst (:any) %17: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %18: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %21 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:        StoreFrameInst %21: any, [i]: any|empty
// CHECK-NEXT:        StoreStackInst false: boolean, %2: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
