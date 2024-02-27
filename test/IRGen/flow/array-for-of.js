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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %3: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: object
// CHECK-NEXT:       StoreStackInst %5: any, %1: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function ""(exports: any): any
// CHECK-NEXT:frame = [exports: any, main: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [exports]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %main(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [main]: any
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function main(x: object): any [typed]
// CHECK-NEXT:frame = [x: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %main(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:object) %x: object
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:object) %5: any, type(object)
// CHECK-NEXT:  %7 = AllocStackInst (:number) $?anon_0_forOfIndex: any
// CHECK-NEXT:       StoreStackInst 0: number, %7: number
// CHECK-NEXT:  %9 = LoadStackInst (:number) %7: number
// CHECK-NEXT:  %10 = FastArrayLengthInst (:number) %6: object
// CHECK-NEXT:        CmpBrLessThanInst %9: number, %10: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = LoadStackInst (:number) %7: number
// CHECK-NEXT:  %13 = FastArrayLoadInst (:number) %6: object, %12: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: number, [i]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %16 = BinaryStrictlyEqualInst (:boolean) %15: any, 0: number
// CHECK-NEXT:        CondBranchInst %16: boolean, %BB5, %BB6
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %19 = LoadStackInst (:number) %7: number
// CHECK-NEXT:  %20 = FastArrayLengthInst (:number) %6: object
// CHECK-NEXT:        CmpBrLessThanInst %19: number, %20: number, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %22 = LoadStackInst (:number) %7: number
// CHECK-NEXT:  %23 = FAddInst (:number) %22: number, 1: number
// CHECK-NEXT:        StoreStackInst %23: number, %7: number
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %28 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %29 = BinaryStrictlyEqualInst (:boolean) %28: any, 1: number
// CHECK-NEXT:        CondBranchInst %29: boolean, %BB8, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %33 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %34 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %35 = CallInst (:any) %33: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %34: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:function_end
