/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

return function main(x: number[]) {
  for (const i of x) {
    if (i === 0) break;
    if (i === 1) continue;
    print(i);
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %""(): functionCode
// CHECK-NEXT:  %3 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %4 = CallInst [njsf] (:any) %2: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: object
// CHECK-NEXT:       StoreStackInst %4: any, %0: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %0: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function ""(exports: any): any
// CHECK-NEXT:frame = [exports: any, main: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %main(): functionCode
// CHECK-NEXT:       StoreFrameInst %2: object, [main]: any
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function main(x: object): any [typed]
// CHECK-NEXT:frame = [x: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %x: object
// CHECK-NEXT:       StoreFrameInst %0: object, [x]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %4 = CheckedTypeCastInst (:object) %3: any, type(object)
// CHECK-NEXT:  %5 = AllocStackInst (:number) $?anon_0_forOfIndex: any
// CHECK-NEXT:       StoreStackInst 0: number, %5: number
// CHECK-NEXT:  %7 = LoadStackInst (:number) %5: number
// CHECK-NEXT:  %8 = FastArrayLengthInst (:number) %4: object
// CHECK-NEXT:       CmpBrLessThanInst %7: number, %8: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadStackInst (:number) %5: number
// CHECK-NEXT:  %11 = FastArrayLoadInst (:number) %4: object, %10: number
// CHECK-NEXT:        StoreFrameInst %11: number, [i]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %14 = BinaryStrictlyEqualInst (:boolean) %13: any, 0: number
// CHECK-NEXT:        CondBranchInst %14: boolean, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = LoadStackInst (:number) %5: number
// CHECK-NEXT:  %18 = FastArrayLengthInst (:number) %4: object
// CHECK-NEXT:        CmpBrLessThanInst %17: number, %18: number, %BB1, %BB2
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %20 = LoadStackInst (:number) %5: number
// CHECK-NEXT:  %21 = FAddInst (:number) %20: number, 1: number
// CHECK-NEXT:        StoreStackInst %21: number, %5: number
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %26 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %27 = BinaryStrictlyEqualInst (:boolean) %26: any, 1: number
// CHECK-NEXT:        CondBranchInst %27: boolean, %BB8, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %31 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %32 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %33 = CallInst (:any) %31: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %32: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:function_end
