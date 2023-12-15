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
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %""(): any
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
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %main(): any
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
// CHECK-NEXT:  %4 = AllocStackInst (:number) $?anon_0_forOfIndex: any
// CHECK-NEXT:       StoreStackInst 0: number, %4: number
// CHECK-NEXT:  %6 = LoadStackInst (:number) %4: number
// CHECK-NEXT:  %7 = FastArrayLengthInst (:number) %3: any
// CHECK-NEXT:       CmpBrLessThanInst %6: number, %7: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = LoadStackInst (:number) %4: number
// CHECK-NEXT:  %10 = FastArrayLoadInst (:number) %3: any, %9: number
// CHECK-NEXT:        StoreFrameInst %10: number, [i]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %13 = BinaryStrictlyEqualInst (:any) %12: any, 0: number
// CHECK-NEXT:        CondBranchInst %13: any, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %16 = LoadStackInst (:number) %4: number
// CHECK-NEXT:  %17 = FastArrayLengthInst (:number) %3: any
// CHECK-NEXT:        CmpBrLessThanInst %16: number, %17: number, %BB1, %BB2
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %19 = LoadStackInst (:number) %4: number
// CHECK-NEXT:  %20 = FAddInst (:number) %19: number, 1: number
// CHECK-NEXT:        StoreStackInst %20: number, %4: number
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %25 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %26 = BinaryStrictlyEqualInst (:any) %25: any, 1: number
// CHECK-NEXT:        CondBranchInst %26: any, %BB8, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %30 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %31 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %32 = CallInst (:any) %30: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %31: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:function_end
