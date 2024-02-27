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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:; <stdin>:18:1
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:source location: [<stdin>:10:1 ... <stdin>:18:2)
// CHECK-NEXT:%BB0:
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [a]: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %4 = LoadParamInst (:any) %b: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [b]: any
// CHECK-NEXT:; <stdin>:11:9
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:; <stdin>:11:13
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:; <stdin>:11:9
// CHECK-NEXT:  %8 = BinaryGreaterThanInst (:boolean) %6: any, %7: any
// CHECK-NEXT:; <stdin>:11:5
// CHECK-NEXT:       CondBranchInst %8: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:; <stdin>:12:9
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:; <stdin>:12:14
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:; <stdin>:12:11
// CHECK-NEXT:  %12 = BinarySubtractInst (:any) %10: any, %11: any
// CHECK-NEXT:; <stdin>:12:11
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: any, [a]: any
// CHECK-NEXT:; <stdin>:13:9
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:; <stdin>:13:15
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:; <stdin>:13:14
// CHECK-NEXT:  %16 = CallInst (:any) %14: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %15: any
// CHECK-NEXT:; <stdin>:11:5
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:; <stdin>:15:9
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:; <stdin>:15:14
// CHECK-NEXT:  %19 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:; <stdin>:15:11
// CHECK-NEXT:  %20 = BinarySubtractInst (:any) %18: any, %19: any
// CHECK-NEXT:; <stdin>:15:11
// CHECK-NEXT:        StoreFrameInst %1: environment, %20: any, [b]: any
// CHECK-NEXT:; <stdin>:16:9
// CHECK-NEXT:  %22 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:; <stdin>:16:15
// CHECK-NEXT:  %23 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:; <stdin>:16:14
// CHECK-NEXT:  %24 = CallInst (:any) %22: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %23: any
// CHECK-NEXT:; <stdin>:11:5
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:; <stdin>:18:1
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
