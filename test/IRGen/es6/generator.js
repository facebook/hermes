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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "simple": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "useResult": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "loop": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "simple2": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "yieldStar": string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "destr": string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "initializer": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:closure) %simple(): any
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7: closure, globalObject: object, "simple": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:closure) %useResult(): any
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9: closure, globalObject: object, "useResult": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:closure) %loop(): any
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11: closure, globalObject: object, "loop": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %14 = StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = CreateFunctionInst (:closure) %simple2(): any
// CHECK-NEXT:  %16 = StorePropertyLooseInst %15: closure, globalObject: object, "simple2": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:closure) %yieldStar(): any
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17: closure, globalObject: object, "yieldStar": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:closure) %destr(): any
// CHECK-NEXT:  %20 = StorePropertyLooseInst %19: closure, globalObject: object, "destr": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:closure) %initializer(): any
// CHECK-NEXT:  %22 = StorePropertyLooseInst %21: closure, globalObject: object, "initializer": string
// CHECK-NEXT:  %23 = LoadStackInst (:any) %13: any
// CHECK-NEXT:  %24 = ReturnInst %23: any
// CHECK-NEXT:function_end

// CHECK:function simple(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_simple(): any
// CHECK-NEXT:  %1 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function useResult(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_useResult(): any
// CHECK-NEXT:  %1 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function loop(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_loop(): any
// CHECK-NEXT:  %1 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function simple2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_simple2(): any
// CHECK-NEXT:  %1 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function yieldStar(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_yieldStar(): any
// CHECK-NEXT:  %1 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function destr(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_destr(): any
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "next": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, %0: object
// CHECK-NEXT:  %3 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function initializer(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_initializer(): any
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "next": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, %0: object
// CHECK-NEXT:  %3 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simple(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:  %4 = CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:  %6 = SaveAndYieldInst 1: number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = ResumeGeneratorInst (:any) %5: boolean
// CHECK-NEXT:  %9 = LoadStackInst (:boolean) %5: boolean
// CHECK-NEXT:  %10 = CondBranchInst %9: boolean, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_useResult(): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:  %4 = CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %6 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:  %7 = SaveAndYieldInst 1: number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ResumeGeneratorInst (:any) %6: boolean
// CHECK-NEXT:  %10 = LoadStackInst (:boolean) %6: boolean
// CHECK-NEXT:  %11 = CondBranchInst %10: boolean, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = StoreFrameInst %9: any, [x]: any
// CHECK-NEXT:  %13 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_loop(x: any): any
// CHECK-NEXT:frame = [x: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:  %4 = CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %6 = StoreFrameInst %5: any, [x]: any
// CHECK-NEXT:  %7 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %8 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %9 = CondBranchInst true: boolean, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %13 = AsNumericInst (:number|bigint) %12: any
// CHECK-NEXT:  %14 = UnaryIncInst (:any) %13: number|bigint
// CHECK-NEXT:  %15 = StoreFrameInst %14: any, [i]: any
// CHECK-NEXT:  %16 = LoadPropertyInst (:any) %11: any, %13: number|bigint
// CHECK-NEXT:  %17 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:  %18 = SaveAndYieldInst %16: any, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %20 = CondBranchInst true: boolean, %BB3, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %21 = ResumeGeneratorInst (:any) %17: boolean
// CHECK-NEXT:  %22 = LoadStackInst (:boolean) %17: boolean
// CHECK-NEXT:  %23 = CondBranchInst %22: boolean, %BB7, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %24 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %25 = ReturnInst %21: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simple2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:  %4 = CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:  %6 = SaveAndYieldInst 1: number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = ResumeGeneratorInst (:any) %5: boolean
// CHECK-NEXT:  %9 = LoadStackInst (:boolean) %5: boolean
// CHECK-NEXT:  %10 = CondBranchInst %9: boolean, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_yieldStar(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:  %4 = CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %7 = GetBuiltinClosureInst (:closure) [globalThis.Symbol]: number
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: closure, "iterator": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %6: any, %8: any
// CHECK-NEXT:  %10 = CallInst (:any) %9: any, empty: any, empty: any, %6: any
// CHECK-NEXT:  %11 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, %10: any, "iterator is not an object": string
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %10: any, "next": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_1_received: any
// CHECK-NEXT:  %14 = StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = AllocStackInst (:boolean) $?anon_2_isReturn: any
// CHECK-NEXT:  %16 = AllocStackInst (:any) $?anon_3_result: any
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %18 = ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %19 = LoadStackInst (:any) %13: any
// CHECK-NEXT:  %20 = CallInst (:any) %12: any, empty: any, empty: any, %10: any, %19: any
// CHECK-NEXT:  %21 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, %20: any, "iterator.next() did not return an object": string
// CHECK-NEXT:  %22 = StoreStackInst %20: any, %16: any
// CHECK-NEXT:  %23 = LoadPropertyInst (:any) %20: any, "done": string
// CHECK-NEXT:  %24 = CondBranchInst %23: any, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %25 = TryStartInst %BB6, %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %26 = LoadStackInst (:any) %16: any
// CHECK-NEXT:  %27 = LoadPropertyInst (:any) %26: any, "value": string
// CHECK-NEXT:  %28 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %29 = ResumeGeneratorInst (:any) %15: boolean
// CHECK-NEXT:  %30 = StoreStackInst %29: any, %13: any
// CHECK-NEXT:  %31 = LoadStackInst (:boolean) %15: boolean
// CHECK-NEXT:  %32 = CondBranchInst %31: boolean, %BB9, %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %33 = CatchInst (:any)
// CHECK-NEXT:  %34 = CallBuiltinInst (:any) [HermesBuiltin.getMethod]: number, empty: any, empty: any, undefined: undefined, %10: any, "throw": string
// CHECK-NEXT:  %35 = CmpBrStrictlyEqualInst %34: any, undefined: undefined, %BB10, %BB11
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %36 = CallBuiltinInst (:any) [HermesBuiltin.generatorSetDelegated]: number, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %37 = SaveAndYieldInst %20: any, %BB8
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %38 = StoreStackInst %29: any, %13: any
// CHECK-NEXT:  %39 = BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %40 = TryEndInst
// CHECK-NEXT:  %41 = CallBuiltinInst (:any) [HermesBuiltin.getMethod]: number, empty: any, empty: any, undefined: undefined, %10: any, "return": string
// CHECK-NEXT:  %42 = CmpBrStrictlyEqualInst %41: any, undefined: undefined, %BB13, %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %43 = LoadStackInst (:any) %13: any
// CHECK-NEXT:  %44 = CallInst (:any) %41: any, empty: any, empty: any, %10: any, %43: any
// CHECK-NEXT:  %45 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, %44: any, "iterator.return() did not return an object": string
// CHECK-NEXT:  %46 = LoadPropertyInst (:any) %44: any, "done": string
// CHECK-NEXT:  %47 = CondBranchInst %46: any, %BB15, %BB16
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %48 = ReturnInst %29: any
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %49 = LoadPropertyInst (:any) %44: any, "value": string
// CHECK-NEXT:  %50 = ReturnInst %49: any
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %51 = CallBuiltinInst (:any) [HermesBuiltin.generatorSetDelegated]: number, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %52 = SaveAndYieldInst %44: any, %BB8
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %53 = BranchInst %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %54 = TryEndInst
// CHECK-NEXT:  %55 = BranchInst %BB3
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %56 = CallInst (:any) %34: any, empty: any, empty: any, %10: any, %33: any
// CHECK-NEXT:  %57 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, %56: any, "iterator.throw() did not return an object": string
// CHECK-NEXT:  %58 = LoadPropertyInst (:any) %56: any, "done": string
// CHECK-NEXT:  %59 = CondBranchInst %58: any, %BB19, %BB20
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %60 = CallBuiltinInst (:any) [HermesBuiltin.getMethod]: number, empty: any, empty: any, undefined: undefined, %10: any, "return": string
// CHECK-NEXT:  %61 = CmpBrStrictlyEqualInst %60: any, undefined: undefined, %BB21, %BB22
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %62 = StoreStackInst %56: any, %16: any
// CHECK-NEXT:  %63 = BranchInst %BB4
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %64 = CallBuiltinInst (:any) [HermesBuiltin.generatorSetDelegated]: number, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %65 = SaveAndYieldInst %56: any, %BB8
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %66 = CallInst (:any) %60: any, empty: any, empty: any, %10: any
// CHECK-NEXT:  %67 = CallBuiltinInst (:any) [HermesBuiltin.ensureObject]: number, empty: any, empty: any, undefined: undefined, %66: any, "iterator.return() did not return an object": string
// CHECK-NEXT:  %68 = BranchInst %BB21
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %69 = CallBuiltinInst (:any) [HermesBuiltin.throwTypeError]: number, empty: any, empty: any, undefined: undefined, "yield* delegate must have a .throw() method": string
// CHECK-NEXT:  %70 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_destr(?anon_2_param: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:  %4 = CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst (:boolean) $?anon_1_isReturn_entry: any
// CHECK-NEXT:  %6 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %7 = LoadParamInst (:any) %?anon_2_param: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_3_iter: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_4_sourceOrNext: any
// CHECK-NEXT:  %10 = StoreStackInst %7: any, %9: any
// CHECK-NEXT:  %11 = IteratorBeginInst (:any) %9: any
// CHECK-NEXT:  %12 = StoreStackInst %11: any, %8: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_5_iterDone: any
// CHECK-NEXT:  %14 = StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_6_iterValue: any
// CHECK-NEXT:  %16 = StoreStackInst undefined: undefined, %15: any
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %18 = ReturnInst %2: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = ResumeGeneratorInst (:any) %5: boolean
// CHECK-NEXT:  %20 = LoadStackInst (:boolean) %5: boolean
// CHECK-NEXT:  %21 = CondBranchInst %20: boolean, %BB5, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %22 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %23 = IteratorNextInst (:any) %8: any, %22: any
// CHECK-NEXT:  %24 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %25 = BinaryStrictlyEqualInst (:any) %24: any, undefined: undefined
// CHECK-NEXT:  %26 = StoreStackInst %25: any, %13: any
// CHECK-NEXT:  %27 = CondBranchInst %25: any, %BB7, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %28 = StoreStackInst %23: any, %15: any
// CHECK-NEXT:  %29 = BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %30 = LoadStackInst (:any) %15: any
// CHECK-NEXT:  %31 = StoreFrameInst %30: any, [x]: any
// CHECK-NEXT:  %32 = LoadStackInst (:any) %13: any
// CHECK-NEXT:  %33 = CondBranchInst %32: any, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %34 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %35 = IteratorCloseInst (:any) %34: any, false: boolean
// CHECK-NEXT:  %36 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %37 = SaveAndYieldInst undefined: undefined, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %38 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %39 = AllocStackInst (:boolean) $?anon_8_isReturn: any
// CHECK-NEXT:  %40 = SaveAndYieldInst %38: any, %BB11
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %41 = ReturnInst %19: any
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %42 = ResumeGeneratorInst (:any) %39: boolean
// CHECK-NEXT:  %43 = LoadStackInst (:boolean) %39: boolean
// CHECK-NEXT:  %44 = CondBranchInst %43: boolean, %BB12, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %45 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %46 = ReturnInst %42: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_initializer(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:  %4 = CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst (:boolean) $?anon_1_isReturn_entry: any
// CHECK-NEXT:  %6 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %7 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %8 = BinaryStrictlyNotEqualInst (:any) %7: any, undefined: undefined
// CHECK-NEXT:  %9 = CondBranchInst %8: any, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst %2: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = ResumeGeneratorInst (:any) %5: boolean
// CHECK-NEXT:  %12 = LoadStackInst (:boolean) %5: boolean
// CHECK-NEXT:  %13 = CondBranchInst %12: boolean, %BB6, %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %15 = CallInst (:any) %14: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %16 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %17 = PhiInst (:any) %7: any, %BB2, %15: any, %BB4
// CHECK-NEXT:  %18 = StoreFrameInst %17: any, [x]: any
// CHECK-NEXT:  %19 = SaveAndYieldInst undefined: undefined, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %20 = AllocStackInst (:boolean) $?anon_2_isReturn: any
// CHECK-NEXT:  %21 = SaveAndYieldInst 1: number, %BB8
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %22 = ReturnInst %11: any
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %23 = ResumeGeneratorInst (:any) %20: boolean
// CHECK-NEXT:  %24 = LoadStackInst (:boolean) %20: boolean
// CHECK-NEXT:  %25 = CondBranchInst %24: boolean, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %26 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %27 = ReturnInst %23: any
// CHECK-NEXT:function_end
