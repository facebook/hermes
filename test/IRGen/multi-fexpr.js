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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "test_fexpr_in_while_cond": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_captured_fexpr_in_while": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_captured_fexpr_in_finally": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_captured_let_in_finally": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %test_fexpr_in_while_cond(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "test_fexpr_in_while_cond": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %test_captured_fexpr_in_while(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "test_captured_fexpr_in_while": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %test_captured_fexpr_in_finally(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "test_captured_fexpr_in_finally": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %test_captured_let_in_finally(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "test_captured_let_in_finally": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function test_fexpr_in_while_cond(): any
// CHECK-NEXT:frame = [f: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_fexpr_in_while_cond(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %f(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [f]: any
// CHECK-NEXT:       CondBranchInst %2: object, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %f(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: object, [f]: any
// CHECK-NEXT:       CondBranchInst %7: object, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:function test_captured_fexpr_in_while(): any
// CHECK-NEXT:frame = [fexpr: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_captured_fexpr_in_while(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %fexpr(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [fexpr]: any
// CHECK-NEXT:  %4 = CallInst (:any) %2: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       CondBranchInst %4: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %fexpr(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: object, [fexpr]: any
// CHECK-NEXT:  %10 = CallInst (:any) %8: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        CondBranchInst %10: any, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:function test_captured_fexpr_in_finally(): any
// CHECK-NEXT:frame = [fexpr: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_captured_fexpr_in_finally(): any, %0: environment
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %"fexpr 1#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [fexpr]: any
// CHECK-NEXT:  %6 = CallInst (:any) %4: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ThrowInst %3: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %10 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %1: environment, %"fexpr 1#"(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: object, [fexpr]: any
// CHECK-NEXT:  %15 = CallInst (:any) %13: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function test_captured_let_in_finally(): any
// CHECK-NEXT:frame = [x: any, fexpr: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_captured_let_in_finally(): any, %0: environment
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:       StorePropertyLooseInst 0: number, globalObject: object, "glob": string
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %"fexpr 2#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: object, [fexpr]: any
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "glob": string
// CHECK-NEXT:        ThrowInst %3: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:        StorePropertyLooseInst 0: number, globalObject: object, "glob": string
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %1: environment, %"fexpr 2#"(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %17: object, [fexpr]: any
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "glob": string
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function f(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %test_fexpr_in_while_cond(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f(): any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function fexpr(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %test_captured_fexpr_in_while(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %fexpr(): any, %0: environment
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "cond": string
// CHECK-NEXT:       CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ResolveScopeInst (:environment) %test_captured_fexpr_in_while(): any, %1: environment
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %4: environment, [fexpr@test_captured_fexpr_in_while]: any
// CHECK-NEXT:       StorePropertyLooseInst %5: any, globalObject: object, "globalFunc": string
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "something": string
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:function "fexpr 1#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %test_captured_fexpr_in_finally(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"fexpr 1#"(): any, %0: environment
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "glob": string
// CHECK-NEXT:       CondBranchInst %2: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ResolveScopeInst (:environment) %test_captured_fexpr_in_finally(): any, %1: environment
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %4: environment, [fexpr@test_captured_fexpr_in_finally]: any
// CHECK-NEXT:       StorePropertyLooseInst %5: any, globalObject: object, "glob": string
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "fexpr 2#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %test_captured_let_in_finally(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"fexpr 2#"(): any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %test_captured_let_in_finally(): any, %1: environment
// CHECK-NEXT:       StoreFrameInst %2: environment, 10: number, [x@test_captured_let_in_finally]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
