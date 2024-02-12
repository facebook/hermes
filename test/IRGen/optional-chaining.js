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
// CHECK-NEXT:       DeclareGlobalVarInst "f1": string
// CHECK-NEXT:       DeclareGlobalVarInst "f2": string
// CHECK-NEXT:       DeclareGlobalVarInst "f3": string
// CHECK-NEXT:       DeclareGlobalVarInst "f4": string
// CHECK-NEXT:       DeclareGlobalVarInst "f5": string
// CHECK-NEXT:       DeclareGlobalVarInst "f6": string
// CHECK-NEXT:       DeclareGlobalVarInst "f7": string
// CHECK-NEXT:       DeclareGlobalVarInst "f8": string
// CHECK-NEXT:       DeclareGlobalVarInst "f9": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %f1(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "f1": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %f2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "f2": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %f3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "f3": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %f4(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "f4": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %f5(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "f5": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %f6(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %19: object, globalObject: object, "f6": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %f7(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %21: object, globalObject: object, "f7": string
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %f8(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %23: object, globalObject: object, "f8": string
// CHECK-NEXT:  %25 = CreateFunctionInst (:object) %f9(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %25: object, globalObject: object, "f9": string
// CHECK-NEXT:  %27 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %27: any
// CHECK-NEXT:  %29 = LoadStackInst (:any) %27: any
// CHECK-NEXT:        ReturnInst %29: any
// CHECK-NEXT:function_end

// CHECK:function f1(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:       CondBranchInst %3: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB2, %8: any, %BB3
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %2: any, "b": string
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function f2(f: any): any
// CHECK-NEXT:frame = [f: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %f: any
// CHECK-NEXT:       StoreFrameInst %0: any, [f]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [f]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:       CondBranchInst %3: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB2, %8: any, %BB3
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function f3(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:       CondBranchInst %3: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB2, %9: any, %BB3
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %2: any, "b": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "c": string
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function f4(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:       CondBranchInst %3: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB2, %10: any, %BB3
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %2: any, "b": string
// CHECK-NEXT:  %9 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined, %2: any
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "c": string
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function f5(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:       CondBranchInst %3: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB2, %9: any, %BB3
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "b": string
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function f6(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:       CondBranchInst %3: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB2, %10: any, %BB3
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "d": string
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %2: any, "b": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "c": string
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function f7(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:       CondBranchInst %3: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB2, %12: any, %BB4
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %2: any, "b": string
// CHECK-NEXT:  %9 = BinaryEqualInst (:any) %8: any, null: null
// CHECK-NEXT:        CondBranchInst %9: any, %BB2, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined, %2: any
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %11: any, "c": string
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function f8(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:       CondBranchInst %3: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB2, %14: any, %BB5
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %2: any, "b": string
// CHECK-NEXT:  %9 = BinaryEqualInst (:any) %8: any, null: null
// CHECK-NEXT:        CondBranchInst %9: any, %BB2, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) %8: any, "c": string
// CHECK-NEXT:  %12 = BinaryEqualInst (:any) %11: any, null: null
// CHECK-NEXT:        CondBranchInst %12: any, %BB2, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = CallInst (:any) %11: any, empty: any, empty: any, undefined: undefined, %8: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function f9(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %3 = BinaryEqualInst (:any) %2: any, null: null
// CHECK-NEXT:       CondBranchInst %3: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst (:any) undefined: undefined, %BB2, %8: any, %BB3
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = DeletePropertyLooseInst (:any) %2: any, "b": string
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:function_end
