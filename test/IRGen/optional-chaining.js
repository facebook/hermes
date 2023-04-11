/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function f1(a) {
  return a?.b;
}

function f2(f) {
  return f?.();
}

function f3(a) {
  return a?.b.c;
}

function f4(a) {
  return a?.b().c;
}

function f5(a) {
  return a?.().b;
}

function f6(a) {
  return (a?.b.c).d;
}

function f7(a) {
  return a?.b?.().c;
}

function f8(a) {
  return a?.b?.c?.();
}

function f9(a) {
  return delete a?.b;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "f1": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "f2": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "f3": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "f4": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "f5": string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "f6": string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "f7": string
// CHECK-NEXT:  %7 = DeclareGlobalVarInst "f8": string
// CHECK-NEXT:  %8 = DeclareGlobalVarInst "f9": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %f1(): any
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9: object, globalObject: object, "f1": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %f2(): any
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11: object, globalObject: object, "f2": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %f3(): any
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13: object, globalObject: object, "f3": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %f4(): any
// CHECK-NEXT:  %16 = StorePropertyLooseInst %15: object, globalObject: object, "f4": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %f5(): any
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17: object, globalObject: object, "f5": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %f6(): any
// CHECK-NEXT:  %20 = StorePropertyLooseInst %19: object, globalObject: object, "f6": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %f7(): any
// CHECK-NEXT:  %22 = StorePropertyLooseInst %21: object, globalObject: object, "f7": string
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %f8(): any
// CHECK-NEXT:  %24 = StorePropertyLooseInst %23: object, globalObject: object, "f8": string
// CHECK-NEXT:  %25 = CreateFunctionInst (:object) %f9(): any
// CHECK-NEXT:  %26 = StorePropertyLooseInst %25: object, globalObject: object, "f9": string
// CHECK-NEXT:  %27 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %28 = StoreStackInst undefined: undefined, %27: any
// CHECK-NEXT:  %29 = LoadStackInst (:any) %27: any
// CHECK-NEXT:  %30 = ReturnInst %29: any
// CHECK-NEXT:function_end

// CHECK:function f1(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB1, %8: any, %BB2
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %2: any, "b": string
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f2(f: any): any
// CHECK-NEXT:frame = [f: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %f: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [f]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [f]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB1, %8: any, %BB2
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f3(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB1, %9: any, %BB2
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %2: any, "b": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "c": string
// CHECK-NEXT:  %10 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f4(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB1, %10: any, %BB2
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %2: any, "b": string
// CHECK-NEXT:  %9 = CallInst (:any) %8: any, empty: any, empty: any, %2: any
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "c": string
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f5(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB1, %9: any, %BB2
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "b": string
// CHECK-NEXT:  %10 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f6(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB1, %10: any, %BB2
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "d": string
// CHECK-NEXT:  %7 = ReturnInst %6: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %2: any, "b": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "c": string
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f7(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB1, %12: any, %BB4
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %2: any, "b": string
// CHECK-NEXT:  %9 = BinaryEqualInst (:any) %8: any, null: null
// CHECK-NEXT:  %10 = CondBranchInst %9: any, %BB1, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = CallInst (:any) %8: any, empty: any, empty: any, %2: any
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %11: any, "c": string
// CHECK-NEXT:  %13 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f8(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB1, %14: any, %BB4
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %2: any, "b": string
// CHECK-NEXT:  %9 = BinaryEqualInst (:any) %8: any, null: null
// CHECK-NEXT:  %10 = CondBranchInst %9: any, %BB1, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) %8: any, "c": string
// CHECK-NEXT:  %12 = BinaryEqualInst (:any) %11: any, null: null
// CHECK-NEXT:  %13 = CondBranchInst %12: any, %BB1, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = CallInst (:any) %11: any, empty: any, empty: any, %8: any
// CHECK-NEXT:  %15 = BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %16 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f9(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB1, %8: any, %BB2
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = DeletePropertyLooseInst (:any) %2: any, "b": string
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
