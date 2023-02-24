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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "test_fexpr_in_while_cond": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "test_captured_fexpr_in_while": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "test_captured_fexpr_in_finally": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "test_captured_let_in_finally": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %test_fexpr_in_while_cond(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "test_fexpr_in_while_cond": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %test_captured_fexpr_in_while(): any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: closure, globalObject: object, "test_captured_fexpr_in_while": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:closure) %test_captured_fexpr_in_finally(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: closure, globalObject: object, "test_captured_fexpr_in_finally": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:closure) %test_captured_let_in_finally(): any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: closure, globalObject: object, "test_captured_let_in_finally": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %13 = StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %15 = ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function test_fexpr_in_while_cond(): any
// CHECK-NEXT:frame = [f: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %f(): any
// CHECK-NEXT:  %1 = StoreFrameInst %0: closure, [f]: any
// CHECK-NEXT:  %2 = CondBranchInst %0: closure, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = CreateFunctionInst (:closure) %f(): any
// CHECK-NEXT:  %6 = StoreFrameInst %5: closure, [f]: any
// CHECK-NEXT:  %7 = CondBranchInst %5: closure, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:function test_captured_fexpr_in_while(): any
// CHECK-NEXT:frame = [fexpr: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %fexpr(): any
// CHECK-NEXT:  %1 = StoreFrameInst %0: closure, [fexpr]: any
// CHECK-NEXT:  %2 = CallInst (:any) %0: closure, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %3 = CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %fexpr(): any
// CHECK-NEXT:  %7 = StoreFrameInst %6: closure, [fexpr]: any
// CHECK-NEXT:  %8 = CallInst (:any) %6: closure, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %9 = CondBranchInst %8: any, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:function test_captured_fexpr_in_finally(): any
// CHECK-NEXT:frame = [fexpr: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = CatchInst (:any)
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %"fexpr 1#"(): any
// CHECK-NEXT:  %3 = StoreFrameInst %2: closure, [fexpr]: any
// CHECK-NEXT:  %4 = CallInst (:any) %2: closure, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %5 = ThrowInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %9 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = TryEndInst
// CHECK-NEXT:  %11 = CreateFunctionInst (:closure) %"fexpr 1#"(): any
// CHECK-NEXT:  %12 = StoreFrameInst %11: closure, [fexpr]: any
// CHECK-NEXT:  %13 = CallInst (:any) %11: closure, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %14 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function test_captured_let_in_finally(): any
// CHECK-NEXT:frame = [x: any, fexpr: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = CatchInst (:any)
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %3 = StorePropertyLooseInst 0: number, globalObject: object, "glob": string
// CHECK-NEXT:  %4 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:closure) %"fexpr 2#"(): any
// CHECK-NEXT:  %6 = StoreFrameInst %5: closure, [fexpr]: any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %5: closure, globalObject: object, "glob": string
// CHECK-NEXT:  %8 = ThrowInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = TryEndInst
// CHECK-NEXT:  %12 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %13 = StorePropertyLooseInst 0: number, globalObject: object, "glob": string
// CHECK-NEXT:  %14 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %15 = CreateFunctionInst (:closure) %"fexpr 2#"(): any
// CHECK-NEXT:  %16 = StoreFrameInst %15: closure, [fexpr]: any
// CHECK-NEXT:  %17 = StorePropertyLooseInst %15: closure, globalObject: object, "glob": string
// CHECK-NEXT:  %18 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function f(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function fexpr(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "cond": string
// CHECK-NEXT:  %1 = CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [fexpr@test_captured_fexpr_in_while]: any
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2: any, globalObject: object, "globalFunc": string
// CHECK-NEXT:  %4 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "something": string
// CHECK-NEXT:  %7 = ReturnInst %6: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "fexpr 1#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "glob": string
// CHECK-NEXT:  %1 = CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [fexpr@test_captured_fexpr_in_finally]: any
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2: any, globalObject: object, "glob": string
// CHECK-NEXT:  %4 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "fexpr 2#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst 10: number, [x@test_captured_let_in_finally]: any
// CHECK-NEXT:  %1 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
