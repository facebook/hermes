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

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %3: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: object
// CHECK-NEXT:       StoreStackInst %5: any, %1: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [exports: any, main: any]

// CHECK:function ""(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.exports]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %main(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS1.main]: any
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [x: any, i: any]

// CHECK:function main(x: object): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:object) %x: object
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [%VS2.x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.i]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [%VS2.x]: any
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:object) %5: any, type(object)
// CHECK-NEXT:  %7 = AllocStackInst (:number) $?anon_0_forOfIndex: any
// CHECK-NEXT:       StoreStackInst 0: number, %7: number
// CHECK-NEXT:  %9 = LoadStackInst (:number) %7: number
// CHECK-NEXT:  %10 = FastArrayLengthInst (:number) %6: object
// CHECK-NEXT:  %11 = FLessThanInst (:boolean) %9: number, %10: number
// CHECK-NEXT:        CondBranchInst %11: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadStackInst (:number) %7: number
// CHECK-NEXT:  %14 = FastArrayLoadInst (:number) %6: object, %13: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: number, [%VS2.i]: any
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [%VS2.i]: any
// CHECK-NEXT:  %17 = BinaryStrictlyEqualInst (:boolean) %16: any, 0: number
// CHECK-NEXT:        CondBranchInst %17: boolean, %BB5, %BB6
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = LoadStackInst (:number) %7: number
// CHECK-NEXT:  %21 = FastArrayLengthInst (:number) %6: object
// CHECK-NEXT:  %22 = FLessThanInst (:boolean) %20: number, %21: number
// CHECK-NEXT:        CondBranchInst %22: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %24 = LoadStackInst (:number) %7: number
// CHECK-NEXT:  %25 = FAddInst (:number) %24: number, 1: number
// CHECK-NEXT:        StoreStackInst %25: number, %7: number
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %30 = LoadFrameInst (:any) %1: environment, [%VS2.i]: any
// CHECK-NEXT:  %31 = BinaryStrictlyEqualInst (:boolean) %30: any, 1: number
// CHECK-NEXT:        CondBranchInst %31: boolean, %BB8, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %35 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %36 = LoadFrameInst (:any) %1: environment, [%VS2.i]: any
// CHECK-NEXT:  %37 = CallInst (:any) %35: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %36: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:function_end
