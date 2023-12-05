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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "simple": string
// CHECK-NEXT:       DeclareGlobalVarInst "useResult": string
// CHECK-NEXT:       DeclareGlobalVarInst "loop": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple2": string
// CHECK-NEXT:       DeclareGlobalVarInst "yieldStar": string
// CHECK-NEXT:       DeclareGlobalVarInst "destr": string
// CHECK-NEXT:       DeclareGlobalVarInst "initializer": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %simple(): any
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "simple": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %useResult(): any
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "useResult": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %loop(): any
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "loop": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %simple2(): any
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "simple2": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %yieldStar(): any
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "yieldStar": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %destr(): any
// CHECK-NEXT:        StorePropertyLooseInst %19: object, globalObject: object, "destr": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %initializer(): any
// CHECK-NEXT:        StorePropertyLooseInst %21: object, globalObject: object, "initializer": string
// CHECK-NEXT:  %23 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %23: any
// CHECK-NEXT:function_end

// CHECK:function simple(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_simple(): any
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function useResult(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_useResult(): any
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function loop(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_loop(): any
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function simple2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_simple2(): any
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function yieldStar(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_yieldStar(): any
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function destr(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_destr(): any
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "next": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function initializer(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_initializer(): any
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "next": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simple(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 1: number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = ResumeGeneratorInst (:any) %5: boolean
// CHECK-NEXT:  %9 = LoadStackInst (:boolean) %5: boolean
// CHECK-NEXT:        CondBranchInst %9: boolean, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_useResult(): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %6 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 1: number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ResumeGeneratorInst (:any) %6: boolean
// CHECK-NEXT:  %10 = LoadStackInst (:boolean) %6: boolean
// CHECK-NEXT:        CondBranchInst %10: boolean, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %9: any, [x]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_loop(x: any): any
// CHECK-NEXT:frame = [x: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %5: any, [x]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:       CondBranchInst true: boolean, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %13 = AsNumericInst (:number|bigint) %12: any
// CHECK-NEXT:  %14 = UnaryIncInst (:any) %13: number|bigint
// CHECK-NEXT:        StoreFrameInst %14: any, [i]: any
// CHECK-NEXT:  %16 = LoadPropertyInst (:any) %11: any, %13: number|bigint
// CHECK-NEXT:  %17 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst %16: any, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        CondBranchInst true: boolean, %BB3, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %21 = ResumeGeneratorInst (:any) %17: boolean
// CHECK-NEXT:  %22 = LoadStackInst (:boolean) %17: boolean
// CHECK-NEXT:        CondBranchInst %22: boolean, %BB7, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        ReturnInst %21: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simple2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 1: number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = ResumeGeneratorInst (:any) %5: boolean
// CHECK-NEXT:  %9 = LoadStackInst (:boolean) %5: boolean
// CHECK-NEXT:        CondBranchInst %9: boolean, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_yieldStar(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %7 = GetBuiltinClosureInst (:object) [globalThis.Symbol]: number
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: object, "iterator": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %6: any, %8: any
// CHECK-NEXT:  %10 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined, %6: any
// CHECK-NEXT:  %11 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %10: any, "iterator is not an object": string
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %10: any, "next": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_1_received: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = AllocStackInst (:boolean) $?anon_2_isReturn: any
// CHECK-NEXT:  %16 = AllocStackInst (:any) $?anon_3_result: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %19 = LoadStackInst (:any) %13: any
// CHECK-NEXT:  %20 = CallInst (:any) %12: any, empty: any, empty: any, undefined: undefined, %10: any, %19: any
// CHECK-NEXT:  %21 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %20: any, "iterator.next() did not return an object": string
// CHECK-NEXT:        StoreStackInst %20: any, %16: any
// CHECK-NEXT:  %23 = LoadPropertyInst (:any) %20: any, "done": string
// CHECK-NEXT:        CondBranchInst %23: any, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        TryStartInst %BB6, %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %26 = LoadStackInst (:any) %16: any
// CHECK-NEXT:  %27 = LoadPropertyInst (:any) %26: any, "value": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %29 = ResumeGeneratorInst (:any) %15: boolean
// CHECK-NEXT:        StoreStackInst %29: any, %13: any
// CHECK-NEXT:  %31 = LoadStackInst (:boolean) %15: boolean
// CHECK-NEXT:        CondBranchInst %31: boolean, %BB9, %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %33 = CatchInst (:any)
// CHECK-NEXT:  %34 = CallBuiltinInst (:any) [HermesBuiltin.getMethod]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %10: any, "throw": string
// CHECK-NEXT:        CmpBrStrictlyEqualInst %34: any, undefined: undefined, %BB10, %BB11
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %36 = CallBuiltinInst (:any) [HermesBuiltin.generatorSetDelegated]: number, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        SaveAndYieldInst %20: any, %BB8
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        StoreStackInst %29: any, %13: any
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %41 = CallBuiltinInst (:any) [HermesBuiltin.getMethod]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %10: any, "return": string
// CHECK-NEXT:        CmpBrStrictlyEqualInst %41: any, undefined: undefined, %BB13, %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %43 = LoadStackInst (:any) %13: any
// CHECK-NEXT:  %44 = CallInst (:any) %41: any, empty: any, empty: any, undefined: undefined, %10: any, %43: any
// CHECK-NEXT:  %45 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %44: any, "iterator.return() did not return an object": string
// CHECK-NEXT:  %46 = LoadPropertyInst (:any) %44: any, "done": string
// CHECK-NEXT:        CondBranchInst %46: any, %BB15, %BB16
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        ReturnInst %29: any
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %49 = LoadPropertyInst (:any) %44: any, "value": string
// CHECK-NEXT:        ReturnInst %49: any
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %51 = CallBuiltinInst (:any) [HermesBuiltin.generatorSetDelegated]: number, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        SaveAndYieldInst %44: any, %BB8
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %53 = CallInst (:any) %34: any, empty: any, empty: any, undefined: undefined, %10: any, %33: any
// CHECK-NEXT:  %54 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %53: any, "iterator.throw() did not return an object": string
// CHECK-NEXT:  %55 = LoadPropertyInst (:any) %53: any, "done": string
// CHECK-NEXT:        CondBranchInst %55: any, %BB17, %BB18
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %57 = CallBuiltinInst (:any) [HermesBuiltin.getMethod]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %10: any, "return": string
// CHECK-NEXT:        CmpBrStrictlyEqualInst %57: any, undefined: undefined, %BB19, %BB20
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        StoreStackInst %53: any, %16: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %61 = CallBuiltinInst (:any) [HermesBuiltin.generatorSetDelegated]: number, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        SaveAndYieldInst %53: any, %BB8
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %63 = CallInst (:any) %57: any, empty: any, empty: any, undefined: undefined, %10: any
// CHECK-NEXT:  %64 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %63: any, "iterator.return() did not return an object": string
// CHECK-NEXT:        BranchInst %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:        ThrowTypeErrorInst "yield* delegate must have a .throw() method": string
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_destr(?anon_2_param: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst (:boolean) $?anon_1_isReturn_entry: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %7 = LoadParamInst (:any) %?anon_2_param: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_3_iter: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_4_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %7: any, %9: any
// CHECK-NEXT:  %11 = IteratorBeginInst (:any) %9: any
// CHECK-NEXT:        StoreStackInst %11: any, %8: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_5_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_6_iterValue: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %15: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst %2: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = ResumeGeneratorInst (:any) %5: boolean
// CHECK-NEXT:  %20 = LoadStackInst (:boolean) %5: boolean
// CHECK-NEXT:        CondBranchInst %20: boolean, %BB5, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %22 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %23 = IteratorNextInst (:any) %8: any, %22: any
// CHECK-NEXT:  %24 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %25 = BinaryStrictlyEqualInst (:any) %24: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %25: any, %13: any
// CHECK-NEXT:        CondBranchInst %25: any, %BB7, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        StoreStackInst %23: any, %15: any
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %30 = LoadStackInst (:any) %15: any
// CHECK-NEXT:        StoreFrameInst %30: any, [x]: any
// CHECK-NEXT:  %32 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        CondBranchInst %32: any, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %34 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %35 = IteratorCloseInst (:any) %34: any, false: boolean
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        SaveAndYieldInst undefined: undefined, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %38 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %39 = AllocStackInst (:boolean) $?anon_8_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst %38: any, %BB11
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %19: any
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %42 = ResumeGeneratorInst (:any) %39: boolean
// CHECK-NEXT:  %43 = LoadStackInst (:boolean) %39: boolean
// CHECK-NEXT:        CondBranchInst %43: boolean, %BB12, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        ReturnInst %42: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_initializer(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst (:boolean) $?anon_1_isReturn_entry: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %7 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %8 = BinaryStrictlyNotEqualInst (:any) %7: any, undefined: undefined
// CHECK-NEXT:       CondBranchInst %8: any, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst %2: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = ResumeGeneratorInst (:any) %5: boolean
// CHECK-NEXT:  %12 = LoadStackInst (:boolean) %5: boolean
// CHECK-NEXT:        CondBranchInst %12: boolean, %BB6, %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %15 = CallInst (:any) %14: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %17 = PhiInst (:any) %7: any, %BB2, %15: any, %BB4
// CHECK-NEXT:        StoreFrameInst %17: any, [x]: any
// CHECK-NEXT:        SaveAndYieldInst undefined: undefined, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %20 = AllocStackInst (:boolean) $?anon_2_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst 1: number, %BB8
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %23 = ResumeGeneratorInst (:any) %20: boolean
// CHECK-NEXT:  %24 = LoadStackInst (:boolean) %20: boolean
// CHECK-NEXT:        CondBranchInst %24: boolean, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        ReturnInst %23: any
// CHECK-NEXT:function_end
