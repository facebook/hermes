/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xenable-tdz -dump-ir -Xes6-block-scoping %s | %FileCheckOrRegen --match-full-lines %s

// Test the optimizer's ability to recover the original optimal loop form if
// IRGen is forced to emit the scoped loop with a flag.

function foo_inline_captures() {
    for(let i = 0; (()=>i < 10)(); (()=>i += 2)()) {
        print((()=>i)());
    }
}

function foo_body_escaping_capture(sink) {
    for(let i = 0; (()=>i < 10)(); (()=>i += 2)()) {
        let f = () => i;
        sink(f);
        print(f());
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo_inline_captures": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_body_escaping_capture": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo_inline_captures(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "foo_inline_captures": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo_body_escaping_capture(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "foo_body_escaping_capture": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo_inline_captures(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = PhiInst (:boolean) true: boolean, %BB0, false: boolean, %BB3
// CHECK-NEXT:  %2 = PhiInst (:number) 0: number, %BB0, %4: number, %BB3
// CHECK-NEXT:       CondBranchInst %1: boolean, %BB2, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = PhiInst (:number) %2: number, %BB1, %11: number, %BB5
// CHECK-NEXT:  %5 = FLessThanInst (:boolean) %4: number, 10: number
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %4: number
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = FAddInst (:number) %2: number, 2: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [i: number]

// CHECK:function foo_body_escaping_capture(sink: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = PhiInst (:boolean) true: boolean, %BB0, false: boolean, %BB3
// CHECK-NEXT:  %3 = PhiInst (:number) 0: number, %BB0, %8: number, %BB3
// CHECK-NEXT:  %4 = CreateScopeInst (:environment) %VS1: any, empty: any
// CHECK-NEXT:       StoreFrameInst %4: environment, %3: number, [%VS1.i]: number
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = PhiInst (:number) %3: number, %BB1, %17: number, %BB5
// CHECK-NEXT:  %8 = PhiInst (:number) %3: number, %BB1, %17: number, %BB5
// CHECK-NEXT:  %9 = FLessThanInst (:boolean) %8: number, 10: number
// CHECK-NEXT:        CondBranchInst %9: boolean, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %4: environment, %VS1: any, %f(): functionCode
// CHECK-NEXT:  %12 = CallInst (:any) %0: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %11: object
// CHECK-NEXT:  %13 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %14 = CallInst (:any) %13: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %7: number
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = FAddInst (:number) %3: number, 2: number
// CHECK-NEXT:        StoreFrameInst %4: environment, %17: number, [%VS1.i]: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:arrow f(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS1.i]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end
