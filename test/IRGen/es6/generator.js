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
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %VS0: any, %simple(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "simple": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %0: environment, %VS0: any, %useResult(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "useResult": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %0: environment, %VS0: any, %loop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "loop": string
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %14: any
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %0: environment, %VS0: any, %simple2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "simple2": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %0: environment, %VS0: any, %yieldStar(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %18: object, globalObject: object, "yieldStar": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %0: environment, %VS0: any, %destr(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %20: object, globalObject: object, "destr": string
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) %0: environment, %VS0: any, %initializer(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %22: object, globalObject: object, "initializer": string
// CHECK-NEXT:  %24 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        ReturnInst %24: any
// CHECK-NEXT:function_end

// CHECK:function simple(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS0: any, %"simple 1#"(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function useResult(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS0: any, %"useResult 1#"(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function loop(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS0: any, %"loop 1#"(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function simple2(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS0: any, %"simple2 1#"(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function yieldStar(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS0: any, %"yieldStar 1#"(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function destr(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS0: any, %"destr 1#"(): functionCode
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: object, "next": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, %2: object
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function initializer(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS0: any, %"initializer 1#"(): functionCode
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: object, "next": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, %2: object
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:scope %VS1 []

// CHECK:generator inner "simple 1#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %5 = CreateScopeInst (:environment) %VS1: any, %4: environment
// CHECK-NEXT:  %6 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 1: number, false: boolean, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ResumeGeneratorInst (:any) %6: boolean
// CHECK-NEXT:  %10 = LoadStackInst (:boolean) %6: boolean
// CHECK-NEXT:        CondBranchInst %10: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [x: any]

// CHECK:generator inner "useResult 1#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %5 = CreateScopeInst (:environment) %VS2: any, %4: environment
// CHECK-NEXT:       StoreFrameInst %5: environment, undefined: undefined, [%VS2.x]: any
// CHECK-NEXT:  %7 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 1: number, false: boolean, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ResumeGeneratorInst (:any) %7: boolean
// CHECK-NEXT:  %11 = LoadStackInst (:boolean) %7: boolean
// CHECK-NEXT:        CondBranchInst %11: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        StoreFrameInst %5: environment, %10: any, [%VS2.x]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [x: any, i: any]

// CHECK:generator inner "loop 1#"(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %5 = CreateScopeInst (:environment) %VS3: any, %4: environment
// CHECK-NEXT:  %6 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %5: environment, %6: any, [%VS3.x]: any
// CHECK-NEXT:       StoreFrameInst %5: environment, undefined: undefined, [%VS3.i]: any
// CHECK-NEXT:       StoreFrameInst %5: environment, 0: number, [%VS3.i]: any
// CHECK-NEXT:        CondBranchInst true: boolean, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %5: environment, [%VS3.x]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %5: environment, [%VS3.i]: any
// CHECK-NEXT:  %14 = AsNumericInst (:number|bigint) %13: any
// CHECK-NEXT:  %15 = UnaryIncInst (:number|bigint) %14: number|bigint
// CHECK-NEXT:        StoreFrameInst %5: environment, %15: number|bigint, [%VS3.i]: any
// CHECK-NEXT:  %17 = LoadPropertyInst (:any) %12: any, %14: number|bigint
// CHECK-NEXT:  %18 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst %17: any, false: boolean, %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        CondBranchInst true: boolean, %BB3, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %22 = ResumeGeneratorInst (:any) %18: boolean
// CHECK-NEXT:  %23 = LoadStackInst (:boolean) %18: boolean
// CHECK-NEXT:        CondBranchInst %23: boolean, %BB8, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        ReturnInst %22: any
// CHECK-NEXT:function_end

// CHECK:scope %VS4 []

// CHECK:generator inner "simple2 1#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %5 = CreateScopeInst (:environment) %VS4: any, %4: environment
// CHECK-NEXT:  %6 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 1: number, false: boolean, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ResumeGeneratorInst (:any) %6: boolean
// CHECK-NEXT:  %10 = LoadStackInst (:boolean) %6: boolean
// CHECK-NEXT:        CondBranchInst %10: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:scope %VS5 []

// CHECK:generator inner "yieldStar 1#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %5 = CreateScopeInst (:environment) %VS5: any, %4: environment
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %8 = GetBuiltinClosureInst (:object) [globalThis.Symbol]: number
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: object, "iterator": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %7: any, %9: any
// CHECK-NEXT:  %11 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, %7: any
// CHECK-NEXT:  %12 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %11: any, "iterator is not an object": string
// CHECK-NEXT:  %13 = LoadPropertyInst (:any) %11: any, "next": string
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_1_received: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %14: any
// CHECK-NEXT:  %16 = AllocStackInst (:boolean) $?anon_2_isReturn: any
// CHECK-NEXT:  %17 = AllocStackInst (:any) $?anon_3_result: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = LoadStackInst (:any) %14: any
// CHECK-NEXT:  %21 = CallInst (:any) %13: any, empty: any, false: boolean, empty: any, undefined: undefined, %11: any, %20: any
// CHECK-NEXT:  %22 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %21: any, "iterator.next() did not return an object": string
// CHECK-NEXT:        StoreStackInst %21: any, %17: any
// CHECK-NEXT:  %24 = LoadPropertyInst (:any) %21: any, "done": string
// CHECK-NEXT:        CondBranchInst %24: any, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        SaveAndYieldInst %21: any, true: boolean, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %27 = LoadStackInst (:any) %17: any
// CHECK-NEXT:  %28 = LoadPropertyInst (:any) %27: any, "value": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryStartInst %BB8, %BB9
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryEndInst %BB8, %BB16
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %32 = CatchInst (:any)
// CHECK-NEXT:  %33 = CallBuiltinInst (:any) [HermesBuiltin.getMethod]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %11: any, "throw": string
// CHECK-NEXT:  %34 = BinaryStrictlyEqualInst (:any) %33: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %34: any, %BB18, %BB17
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %36 = ResumeGeneratorInst (:any) %16: boolean
// CHECK-NEXT:        StoreStackInst %36: any, %14: any
// CHECK-NEXT:  %38 = LoadStackInst (:boolean) %16: boolean
// CHECK-NEXT:        CondBranchInst %38: boolean, %BB10, %BB7
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        StoreStackInst %36: any, %14: any
// CHECK-NEXT:        TryEndInst %BB8, %BB11
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %42 = CallBuiltinInst (:any) [HermesBuiltin.getMethod]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %11: any, "return": string
// CHECK-NEXT:  %43 = BinaryStrictlyEqualInst (:any) %42: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %43: any, %BB13, %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %45 = LoadStackInst (:any) %14: any
// CHECK-NEXT:  %46 = CallInst (:any) %42: any, empty: any, false: boolean, empty: any, undefined: undefined, %11: any, %45: any
// CHECK-NEXT:  %47 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %46: any, "iterator.return() did not return an object": string
// CHECK-NEXT:  %48 = LoadPropertyInst (:any) %46: any, "done": string
// CHECK-NEXT:        CondBranchInst %48: any, %BB14, %BB15
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        ReturnInst %36: any
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %51 = LoadPropertyInst (:any) %46: any, "value": string
// CHECK-NEXT:        ReturnInst %51: any
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        SaveAndYieldInst %46: any, true: boolean, %BB6
// CHECK-NEXT:%BB16:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %55 = CallInst (:any) %33: any, empty: any, false: boolean, empty: any, undefined: undefined, %11: any, %32: any
// CHECK-NEXT:  %56 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %55: any, "iterator.throw() did not return an object": string
// CHECK-NEXT:  %57 = LoadPropertyInst (:any) %55: any, "done": string
// CHECK-NEXT:        CondBranchInst %57: any, %BB19, %BB20
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %59 = CallBuiltinInst (:any) [HermesBuiltin.getMethod]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %11: any, "return": string
// CHECK-NEXT:  %60 = BinaryStrictlyEqualInst (:any) %59: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %60: any, %BB22, %BB21
// CHECK-NEXT:%BB19:
// CHECK-NEXT:        StoreStackInst %55: any, %17: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB20:
// CHECK-NEXT:        SaveAndYieldInst %55: any, true: boolean, %BB6
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %65 = CallInst (:any) %59: any, empty: any, false: boolean, empty: any, undefined: undefined, %11: any
// CHECK-NEXT:  %66 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %65: any, "iterator.return() did not return an object": string
// CHECK-NEXT:        BranchInst %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:        ThrowTypeErrorInst "yield* delegate must have a .throw() method": string
// CHECK-NEXT:function_end

// CHECK:scope %VS6 [x: any]

// CHECK:generator inner "destr 1#"(?anon_2_param: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = AllocStackInst (:boolean) $?anon_1_isReturn_entry: any
// CHECK-NEXT:  %5 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %6 = CreateScopeInst (:environment) %VS6: any, %5: environment
// CHECK-NEXT:       StoreFrameInst %6: environment, undefined: undefined, [%VS6.x]: any
// CHECK-NEXT:  %8 = LoadParamInst (:any) %?anon_2_param: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_3_iter: any
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_4_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %8: any, %10: any
// CHECK-NEXT:  %12 = IteratorBeginInst (:any) %10: any
// CHECK-NEXT:        StoreStackInst %12: any, %9: any
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_5_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %14: any
// CHECK-NEXT:  %16 = AllocStackInst (:any) $?anon_6_iterValue: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %16: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = ResumeGeneratorInst (:any) %4: boolean
// CHECK-NEXT:  %21 = LoadStackInst (:boolean) %4: boolean
// CHECK-NEXT:        CondBranchInst %21: boolean, %BB10, %BB9
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %23 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %24 = IteratorNextInst (:any) %9: any, %23: any
// CHECK-NEXT:  %25 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %26 = BinaryStrictlyEqualInst (:any) %25: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %26: any, %14: any
// CHECK-NEXT:        CondBranchInst %26: any, %BB6, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreStackInst %24: any, %16: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %31 = LoadStackInst (:any) %16: any
// CHECK-NEXT:        StoreFrameInst %6: environment, %31: any, [%VS6.x]: any
// CHECK-NEXT:  %33 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        CondBranchInst %33: any, %BB8, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %35 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %36 = IteratorCloseInst (:any) %35: any, false: boolean
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        SaveAndYieldInst undefined: undefined, false: boolean, %BB3
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %39 = LoadFrameInst (:any) %6: environment, [%VS6.x]: any
// CHECK-NEXT:  %40 = AllocStackInst (:boolean) $?anon_8_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst %39: any, false: boolean, %BB11
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        ReturnInst %20: any
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %43 = ResumeGeneratorInst (:any) %40: boolean
// CHECK-NEXT:  %44 = LoadStackInst (:boolean) %40: boolean
// CHECK-NEXT:        CondBranchInst %44: boolean, %BB13, %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        ReturnInst %43: any
// CHECK-NEXT:function_end

// CHECK:scope %VS7 [x: any]

// CHECK:generator inner "initializer 1#"(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = AllocStackInst (:boolean) $?anon_1_isReturn_entry: any
// CHECK-NEXT:  %5 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %6 = CreateScopeInst (:environment) %VS7: any, %5: environment
// CHECK-NEXT:       StoreFrameInst %6: environment, undefined: undefined, [%VS7.x]: any
// CHECK-NEXT:  %8 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %9 = BinaryStrictlyNotEqualInst (:any) %8: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %9: any, %BB5, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = ResumeGeneratorInst (:any) %4: boolean
// CHECK-NEXT:  %13 = LoadStackInst (:boolean) %4: boolean
// CHECK-NEXT:        CondBranchInst %13: boolean, %BB7, %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %16 = CallInst (:any) %15: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %18 = PhiInst (:any) %8: any, %BB1, %16: any, %BB4
// CHECK-NEXT:        StoreFrameInst %6: environment, %18: any, [%VS7.x]: any
// CHECK-NEXT:        SaveAndYieldInst undefined: undefined, false: boolean, %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %21 = AllocStackInst (:boolean) $?anon_2_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst 1: number, false: boolean, %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %24 = ResumeGeneratorInst (:any) %21: boolean
// CHECK-NEXT:  %25 = LoadStackInst (:boolean) %21: boolean
// CHECK-NEXT:        CondBranchInst %25: boolean, %BB10, %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        ReturnInst %24: any
// CHECK-NEXT:function_end
