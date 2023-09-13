/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O0 | %FileCheckOrRegen --match-full-lines %s

// This file tests IRGen in situations where it compiles the same AST more than once.
// It happens in two cases:
// - When compiling a loop pre-condition, it is compiled both as a pre and
//   post-condition.
// - When compiling a "finally" handler, it is inlined in the exception case
//   and the normal completion case.
//
// There are two important things to test in both of these cases above:
// - That the variables are not rebound again and the same variables are used.
// - That function expressions do not result in more than one IR Function.

function test_fexpr_in_while_cond() {
    while(function f(){}){
    }
}

function test_captured_fexpr_in_while() {
    while ((function fexpr() {
        if (cond) globalFunc = fexpr;
        return something;
       })()) {
    }
}

function test_captured_fexpr_in_finally() {
    try {
        foo();
    } finally {
        (function fexpr(){
            if (!glob)
                glob = fexpr;
        })();
    }
}

function test_captured_let_in_finally() {
    try {
    } finally {
        glob = 0;
        let x;
        glob = function fexpr() { x = 10; }
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "test_fexpr_in_while_cond": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_captured_fexpr_in_while": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_captured_fexpr_in_finally": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_captured_let_in_finally": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %test_fexpr_in_while_cond(): any
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "test_fexpr_in_while_cond": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %test_captured_fexpr_in_while(): any
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "test_captured_fexpr_in_while": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %test_captured_fexpr_in_finally(): any
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "test_captured_fexpr_in_finally": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %test_captured_let_in_finally(): any
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "test_captured_let_in_finally": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function test_fexpr_in_while_cond(): any
// CHECK-NEXT:frame = [f: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %f(): any
// CHECK-NEXT:       StoreFrameInst %0: object, [f]: any
// CHECK-NEXT:       CondBranchInst %0: object, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %f(): any
// CHECK-NEXT:       StoreFrameInst %5: object, [f]: any
// CHECK-NEXT:       CondBranchInst %5: object, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:function test_captured_fexpr_in_while(): any
// CHECK-NEXT:frame = [fexpr: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %fexpr(): any
// CHECK-NEXT:       StoreFrameInst %0: object, [fexpr]: any
// CHECK-NEXT:  %2 = CallInst (:any) %0: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %fexpr(): any
// CHECK-NEXT:       StoreFrameInst %6: object, [fexpr]: any
// CHECK-NEXT:  %8 = CallInst (:any) %6: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       CondBranchInst %8: any, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:function test_captured_fexpr_in_finally(): any
// CHECK-NEXT:frame = [fexpr: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = CatchInst (:any)
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %"fexpr 1#"(): any
// CHECK-NEXT:       StoreFrameInst %2: object, [fexpr]: any
// CHECK-NEXT:  %4 = CallInst (:any) %2: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ThrowInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %"fexpr 1#"(): any
// CHECK-NEXT:        StoreFrameInst %11: object, [fexpr]: any
// CHECK-NEXT:  %13 = CallInst (:any) %11: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function test_captured_let_in_finally(): any
// CHECK-NEXT:frame = [x: any, fexpr: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = CatchInst (:any)
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:       StorePropertyLooseInst 0: number, globalObject: object, "glob": string
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %"fexpr 2#"(): any
// CHECK-NEXT:       StoreFrameInst %5: object, [fexpr]: any
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "glob": string
// CHECK-NEXT:       ThrowInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:        StorePropertyLooseInst 0: number, globalObject: object, "glob": string
// CHECK-NEXT:        StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %"fexpr 2#"(): any
// CHECK-NEXT:        StoreFrameInst %15: object, [fexpr]: any
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "glob": string
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function f(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function fexpr(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "cond": string
// CHECK-NEXT:       CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [fexpr@test_captured_fexpr_in_while]: any
// CHECK-NEXT:       StorePropertyLooseInst %2: any, globalObject: object, "globalFunc": string
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "something": string
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function "fexpr 1#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "glob": string
// CHECK-NEXT:       CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [fexpr@test_captured_fexpr_in_finally]: any
// CHECK-NEXT:       StorePropertyLooseInst %2: any, globalObject: object, "glob": string
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "fexpr 2#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst 10: number, [x@test_captured_let_in_finally]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
