/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc - -O0 -dump-source-location=loc -dump-ir < %s | %FileCheckOrRegen --match-full-lines %s

function foo(a,b) {
    if (a > b) {
        a -= b;
        print(a);
    } else {
        b -= a;
        print(b);
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:source location: [<stdin>:10:1 ... <stdin>:18:2)
// CHECK-NEXT:%BB0:
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:; <stdin>:18:1
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:source location: [<stdin>:10:1 ... <stdin>:18:2)
// CHECK-NEXT:%BB0:
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:       StoreFrameInst %2: any, [b]: any
// CHECK-NEXT:; <stdin>:11:9
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:; <stdin>:11:13
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:; <stdin>:11:9
// CHECK-NEXT:  %6 = BinaryGreaterThanInst (:boolean) %4: any, %5: any
// CHECK-NEXT:; <stdin>:11:5
// CHECK-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:; <stdin>:12:9
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:; <stdin>:12:14
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:; <stdin>:12:11
// CHECK-NEXT:  %10 = BinarySubtractInst (:any) %8: any, %9: any
// CHECK-NEXT:; <stdin>:12:11
// CHECK-NEXT:        StoreFrameInst %10: any, [a]: any
// CHECK-NEXT:; <stdin>:13:9
// CHECK-NEXT:  %12 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:; <stdin>:13:15
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:; <stdin>:13:14
// CHECK-NEXT:  %14 = CallInst (:any) %12: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %13: any
// CHECK-NEXT:; <stdin>:11:5
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:; <stdin>:15:9
// CHECK-NEXT:  %16 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:; <stdin>:15:14
// CHECK-NEXT:  %17 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:; <stdin>:15:11
// CHECK-NEXT:  %18 = BinarySubtractInst (:any) %16: any, %17: any
// CHECK-NEXT:; <stdin>:15:11
// CHECK-NEXT:        StoreFrameInst %18: any, [b]: any
// CHECK-NEXT:; <stdin>:16:9
// CHECK-NEXT:  %20 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:; <stdin>:16:15
// CHECK-NEXT:  %21 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:; <stdin>:16:14
// CHECK-NEXT:  %22 = CallInst (:any) %20: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %21: any
// CHECK-NEXT:; <stdin>:11:5
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:; <stdin>:18:1
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
