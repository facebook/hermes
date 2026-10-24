/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Xdump-functions=main -typed -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermes -Xdump-functions=main -typed -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// join() coerces every element with __ToString, which uses AddEmptyStringInst
// for primitives and only calls the global String() for objects. Requires -O
// so join() and __ToString are inlined into main.
return function main(x: number[]): string {
  return x.join("-");
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function main(x: object): string [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %x: object
// CHECK-NEXT:  %1 = FastArrayLengthInst (:number) %0: object
// CHECK-NEXT:  %2 = FEqualInst (:boolean) %1: number, 0: number
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = PhiInst (:string) %16: string, %BB6, "": string, %BB0
// CHECK-NEXT:       ReturnInst %4: string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = FastArrayLoadInst (:number) %0: object, 0: number
// CHECK-NEXT:  %7 = TypeOfIsInst (:boolean) %6: number, typeOfIs(Object,Null)
// CHECK-NEXT:       CondBranchInst %7: boolean, %BB12, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = TypeOfIsInst (:boolean) %6: number, typeOfIs(Function)
// CHECK-NEXT:        CondBranchInst %9: boolean, %BB12, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = AddEmptyStringInst (:string) %6: number
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %13 = PhiInst (:string) %11: string, %BB4, %38: string, %BB12
// CHECK-NEXT:  %14 = FLessThanInst (:boolean) 1: number, %1: number
// CHECK-NEXT:        CondBranchInst %14: boolean, %BB7, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %16 = PhiInst (:string) %13: string, %BB5, %28: string, %BB10
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %18 = PhiInst (:number) 1: number, %BB5, %29: number, %BB10
// CHECK-NEXT:  %19 = PhiInst (:string) %13: string, %BB5, %28: string, %BB10
// CHECK-NEXT:  %20 = FastArrayLoadInst (:number) %0: object, %18: number
// CHECK-NEXT:  %21 = TypeOfIsInst (:boolean) %20: number, typeOfIs(Object,Null)
// CHECK-NEXT:        CondBranchInst %21: boolean, %BB11, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %23 = TypeOfIsInst (:boolean) %20: number, typeOfIs(Function)
// CHECK-NEXT:        CondBranchInst %23: boolean, %BB11, %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %25 = AddEmptyStringInst (:string) %20: number
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %27 = PhiInst (:string) %25: string, %BB9, %34: string, %BB11
// CHECK-NEXT:  %28 = StringConcatInst (:string) %19: string, "-": string, %27: string
// CHECK-NEXT:  %29 = FAddInst (:number) %18: number, 1: number
// CHECK-NEXT:  %30 = FLessThanInst (:boolean) %29: number, %1: number
// CHECK-NEXT:        CondBranchInst %30: boolean, %BB7, %BB6
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %32 = TryLoadGlobalPropertyInst (:any) globalObject: object, "String": string
// CHECK-NEXT:  %33 = CallInst (:any) %32: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %20: number
// CHECK-NEXT:  %34 = CheckedTypeCastInst (:string) %33: any, type(string)
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %36 = TryLoadGlobalPropertyInst (:any) globalObject: object, "String": string
// CHECK-NEXT:  %37 = CallInst (:any) %36: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %6: number
// CHECK-NEXT:  %38 = CheckedTypeCastInst (:string) %37: any, type(string)
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:function_end
