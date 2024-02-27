/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -typed -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

// Make sure two specializations are generated for number and string.
// TODO: Original "any" version still exists during IRGen,
// should probably just be deleted.
function id<T>(x: T): T {
  return x;
}

id<number>(1);
id<string>('a');

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
// CHECK-NEXT:frame = [exports: any, id: any, id#1: any, id#2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [exports]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %id(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [id#1]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %"id 1#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [id#2]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [id#1]: any
// CHECK-NEXT:  %9 = CheckedTypeCastInst (:object) %8: any, type(object)
// CHECK-NEXT:  %10 = CallInst [njsf] (:any) %9: object, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:number) %10: any, type(number)
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [id#2]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:object) %12: any, type(object)
// CHECK-NEXT:  %14 = CallInst [njsf] (:any) %13: object, empty: any, empty: any, undefined: undefined, undefined: undefined, "a": string
// CHECK-NEXT:  %15 = CheckedTypeCastInst (:string) %14: any, type(string)
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function id(x: number): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %id(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: number, [x]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:number) %4: any, type(number)
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:function "id 1#"(x: string): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"id 1#"(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:string) %x: string
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: string, [x]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:string) %4: any, type(string)
// CHECK-NEXT:       ReturnInst %5: string
// CHECK-NEXT:function_end
