/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function* simple() {
  yield 1;
}

function *useResult() {
  var x = yield 1;
}

function *loop(x) {
  var i = 0;
  while (true) {
    yield x[i++];
  }
}

// Test generation of function expressions.
var simple2 = function*() {
  yield 1;
}

var yieldStar = function*() {
  yield* foo();
}

var destr = function*([x]) {
  yield x;
}

var initializer = function*(x = foo()) {
  yield 1;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "simple": string
// CHECK-NEXT:       DeclareGlobalVarInst "useResult": string
// CHECK-NEXT:       DeclareGlobalVarInst "loop": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple2": string
// CHECK-NEXT:       DeclareGlobalVarInst "yieldStar": string
// CHECK-NEXT:       DeclareGlobalVarInst "destr": string
// CHECK-NEXT:       DeclareGlobalVarInst "initializer": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %simple(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "simple": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %0: environment, %useResult(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "useResult": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %0: environment, %loop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "loop": string
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %14: any
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %0: environment, %simple2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "simple2": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %0: environment, %yieldStar(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %18: object, globalObject: object, "yieldStar": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %0: environment, %destr(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %20: object, globalObject: object, "destr": string
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) %0: environment, %initializer(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %22: object, globalObject: object, "initializer": string
// CHECK-NEXT:  %24 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        ReturnInst %24: any
// CHECK-NEXT:function_end

// CHECK:function simple(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %?anon_0_simple(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function useResult(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %?anon_0_useResult(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function loop(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %?anon_0_loop(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function simple2(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %?anon_0_simple2(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function yieldStar(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %?anon_0_yieldStar(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function destr(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %?anon_0_destr(): functionCode
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: object, "next": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, %2: object
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function initializer(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %?anon_0_initializer(): functionCode
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: object, "next": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, %2: object
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:scope %VS1 []

// CHECK:generator inner ?anon_0_simple(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %6 = CreateScopeInst (:environment) %VS1: any, %5: environment
// CHECK-NEXT:  %7 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 1: number, false: boolean, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ResumeGeneratorInst (:any) %7: boolean
// CHECK-NEXT:  %11 = LoadStackInst (:boolean) %7: boolean
// CHECK-NEXT:        CondBranchInst %11: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [x: any]

// CHECK:generator inner ?anon_0_useResult(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %6 = CreateScopeInst (:environment) %VS2: any, %5: environment
// CHECK-NEXT:       StoreFrameInst %6: environment, undefined: undefined, [%VS2.x]: any
// CHECK-NEXT:  %8 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 1: number, false: boolean, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = ResumeGeneratorInst (:any) %8: boolean
// CHECK-NEXT:  %12 = LoadStackInst (:boolean) %8: boolean
// CHECK-NEXT:        CondBranchInst %12: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        StoreFrameInst %6: environment, %11: any, [%VS2.x]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [x: any, i: any]

// CHECK:generator inner ?anon_0_loop(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %6 = CreateScopeInst (:environment) %VS3: any, %5: environment
// CHECK-NEXT:  %7 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %6: environment, %7: any, [%VS3.x]: any
// CHECK-NEXT:       StoreFrameInst %6: environment, undefined: undefined, [%VS3.i]: any
// CHECK-NEXT:        StoreFrameInst %6: environment, 0: number, [%VS3.i]: any
// CHECK-NEXT:        CondBranchInst true: boolean, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %6: environment, [%VS3.x]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %6: environment, [%VS3.i]: any
// CHECK-NEXT:  %15 = AsNumericInst (:number|bigint) %14: any
// CHECK-NEXT:  %16 = UnaryIncInst (:number|bigint) %15: number|bigint
// CHECK-NEXT:        StoreFrameInst %6: environment, %16: number|bigint, [%VS3.i]: any
// CHECK-NEXT:  %18 = LoadPropertyInst (:any) %13: any, %15: number|bigint
// CHECK-NEXT:  %19 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst %18: any, false: boolean, %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        CondBranchInst true: boolean, %BB3, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %23 = ResumeGeneratorInst (:any) %19: boolean
// CHECK-NEXT:  %24 = LoadStackInst (:boolean) %19: boolean
// CHECK-NEXT:        CondBranchInst %24: boolean, %BB8, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        ReturnInst %23: any
// CHECK-NEXT:function_end

// CHECK:scope %VS4 []

// CHECK:generator inner ?anon_0_simple2(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %6 = CreateScopeInst (:environment) %VS4: any, %5: environment
// CHECK-NEXT:  %7 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 1: number, false: boolean, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ResumeGeneratorInst (:any) %7: boolean
// CHECK-NEXT:  %11 = LoadStackInst (:boolean) %7: boolean
// CHECK-NEXT:        CondBranchInst %11: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end

// CHECK:scope %VS5 []

// CHECK:generator inner ?anon_0_yieldStar(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %6 = CreateScopeInst (:environment) %VS5: any, %5: environment
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %9 = GetBuiltinClosureInst (:object) [globalThis.Symbol]: number
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: object, "iterator": string
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) %8: any, %10: any
// CHECK-NEXT:  %12 = CallInst (:any) %11: any, empty: any, empty: any, undefined: undefined, %8: any
// CHECK-NEXT:  %13 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %12: any, "iterator is not an object": string
// CHECK-NEXT:  %14 = LoadPropertyInst (:any) %12: any, "next": string
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_1_received: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %15: any
// CHECK-NEXT:  %17 = AllocStackInst (:boolean) $?anon_2_isReturn: any
// CHECK-NEXT:  %18 = AllocStackInst (:any) $?anon_3_result: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %21 = LoadStackInst (:any) %15: any
// CHECK-NEXT:  %22 = CallInst (:any) %14: any, empty: any, empty: any, undefined: undefined, %12: any, %21: any
// CHECK-NEXT:  %23 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %22: any, "iterator.next() did not return an object": string
// CHECK-NEXT:        StoreStackInst %22: any, %18: any
// CHECK-NEXT:  %25 = LoadPropertyInst (:any) %22: any, "done": string
// CHECK-NEXT:        CondBranchInst %25: any, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        SaveAndYieldInst %22: any, true: boolean, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %28 = LoadStackInst (:any) %18: any
// CHECK-NEXT:  %29 = LoadPropertyInst (:any) %28: any, "value": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryStartInst %BB8, %BB9
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryEndInst %BB8, %BB16
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %33 = CatchInst (:any)
// CHECK-NEXT:  %34 = CallBuiltinInst (:any) [HermesBuiltin.getMethod]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %12: any, "throw": string
// CHECK-NEXT:        CmpBrStrictlyEqualInst %34: any, undefined: undefined, %BB18, %BB17
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %36 = ResumeGeneratorInst (:any) %17: boolean
// CHECK-NEXT:        StoreStackInst %36: any, %15: any
// CHECK-NEXT:  %38 = LoadStackInst (:boolean) %17: boolean
// CHECK-NEXT:        CondBranchInst %38: boolean, %BB10, %BB7
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        StoreStackInst %36: any, %15: any
// CHECK-NEXT:        TryEndInst %BB8, %BB11
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %42 = CallBuiltinInst (:any) [HermesBuiltin.getMethod]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %12: any, "return": string
// CHECK-NEXT:        CmpBrStrictlyEqualInst %42: any, undefined: undefined, %BB13, %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %44 = LoadStackInst (:any) %15: any
// CHECK-NEXT:  %45 = CallInst (:any) %42: any, empty: any, empty: any, undefined: undefined, %12: any, %44: any
// CHECK-NEXT:  %46 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %45: any, "iterator.return() did not return an object": string
// CHECK-NEXT:  %47 = LoadPropertyInst (:any) %45: any, "done": string
// CHECK-NEXT:        CondBranchInst %47: any, %BB14, %BB15
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        ReturnInst %36: any
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %50 = LoadPropertyInst (:any) %45: any, "value": string
// CHECK-NEXT:        ReturnInst %50: any
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        SaveAndYieldInst %45: any, true: boolean, %BB6
// CHECK-NEXT:%BB16:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %54 = CallInst (:any) %34: any, empty: any, empty: any, undefined: undefined, %12: any, %33: any
// CHECK-NEXT:  %55 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %54: any, "iterator.throw() did not return an object": string
// CHECK-NEXT:  %56 = LoadPropertyInst (:any) %54: any, "done": string
// CHECK-NEXT:        CondBranchInst %56: any, %BB19, %BB20
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %58 = CallBuiltinInst (:any) [HermesBuiltin.getMethod]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %12: any, "return": string
// CHECK-NEXT:        CmpBrStrictlyEqualInst %58: any, undefined: undefined, %BB22, %BB21
// CHECK-NEXT:%BB19:
// CHECK-NEXT:        StoreStackInst %54: any, %18: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB20:
// CHECK-NEXT:        SaveAndYieldInst %54: any, true: boolean, %BB6
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %63 = CallInst (:any) %58: any, empty: any, empty: any, undefined: undefined, %12: any
// CHECK-NEXT:  %64 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %63: any, "iterator.return() did not return an object": string
// CHECK-NEXT:        BranchInst %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:        ThrowTypeErrorInst "yield* delegate must have a .throw() method": string
// CHECK-NEXT:function_end

// CHECK:scope %VS6 [x: any]

// CHECK:generator inner ?anon_0_destr(?anon_2_param: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = AllocStackInst (:boolean) $?anon_1_isReturn_entry: any
// CHECK-NEXT:  %6 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %7 = CreateScopeInst (:environment) %VS6: any, %6: environment
// CHECK-NEXT:       StoreFrameInst %7: environment, undefined: undefined, [%VS6.x]: any
// CHECK-NEXT:  %9 = LoadParamInst (:any) %?anon_2_param: any
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_3_iter: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_4_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %9: any, %11: any
// CHECK-NEXT:  %13 = IteratorBeginInst (:any) %11: any
// CHECK-NEXT:        StoreStackInst %13: any, %10: any
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_5_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %15: any
// CHECK-NEXT:  %17 = AllocStackInst (:any) $?anon_6_iterValue: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %17: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %21 = ResumeGeneratorInst (:any) %5: boolean
// CHECK-NEXT:  %22 = LoadStackInst (:boolean) %5: boolean
// CHECK-NEXT:        CondBranchInst %22: boolean, %BB10, %BB9
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %24 = LoadStackInst (:any) %11: any
// CHECK-NEXT:  %25 = IteratorNextInst (:any) %10: any, %24: any
// CHECK-NEXT:  %26 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %27 = BinaryStrictlyEqualInst (:any) %26: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %27: any, %15: any
// CHECK-NEXT:        CondBranchInst %27: any, %BB6, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreStackInst %25: any, %17: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %32 = LoadStackInst (:any) %17: any
// CHECK-NEXT:        StoreFrameInst %7: environment, %32: any, [%VS6.x]: any
// CHECK-NEXT:  %34 = LoadStackInst (:any) %15: any
// CHECK-NEXT:        CondBranchInst %34: any, %BB8, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %36 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %37 = IteratorCloseInst (:any) %36: any, false: boolean
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        SaveAndYieldInst undefined: undefined, false: boolean, %BB3
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %40 = LoadFrameInst (:any) %7: environment, [%VS6.x]: any
// CHECK-NEXT:  %41 = AllocStackInst (:boolean) $?anon_8_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst %40: any, false: boolean, %BB11
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        ReturnInst %21: any
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %44 = ResumeGeneratorInst (:any) %41: boolean
// CHECK-NEXT:  %45 = LoadStackInst (:boolean) %41: boolean
// CHECK-NEXT:        CondBranchInst %45: boolean, %BB13, %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        ReturnInst %44: any
// CHECK-NEXT:function_end

// CHECK:scope %VS7 [x: any]

// CHECK:generator inner ?anon_0_initializer(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = AllocStackInst (:boolean) $?anon_1_isReturn_entry: any
// CHECK-NEXT:  %6 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %7 = CreateScopeInst (:environment) %VS7: any, %6: environment
// CHECK-NEXT:       StoreFrameInst %7: environment, undefined: undefined, [%VS7.x]: any
// CHECK-NEXT:  %9 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %10 = BinaryStrictlyNotEqualInst (:any) %9: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %10: any, %BB5, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = ResumeGeneratorInst (:any) %5: boolean
// CHECK-NEXT:  %14 = LoadStackInst (:boolean) %5: boolean
// CHECK-NEXT:        CondBranchInst %14: boolean, %BB7, %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %17 = CallInst (:any) %16: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %19 = PhiInst (:any) %9: any, %BB1, %17: any, %BB4
// CHECK-NEXT:        StoreFrameInst %7: environment, %19: any, [%VS7.x]: any
// CHECK-NEXT:        SaveAndYieldInst undefined: undefined, false: boolean, %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %22 = AllocStackInst (:boolean) $?anon_2_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst 1: number, false: boolean, %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %25 = ResumeGeneratorInst (:any) %22: boolean
// CHECK-NEXT:  %26 = LoadStackInst (:boolean) %22: boolean
// CHECK-NEXT:        CondBranchInst %26: boolean, %BB10, %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        ReturnInst %25: any
// CHECK-NEXT:function_end
