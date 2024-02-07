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
// CHECK-NEXT:frame = [exports: any, id: any, id#1: any, id#2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %id(): functionCode
// CHECK-NEXT:       StoreFrameInst %2: object, [id#1]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %"id 1#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %4: object, [id#2]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [id#1]: any
// CHECK-NEXT:  %7 = CheckedTypeCastInst (:object) %6: any, type(object)
// CHECK-NEXT:  %8 = CallInst [njsf] (:any) %7: object, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:  %9 = CheckedTypeCastInst (:number) %8: any, type(number)
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [id#2]: any
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:object) %10: any, type(object)
// CHECK-NEXT:  %12 = CallInst [njsf] (:any) %11: object, empty: any, empty: any, undefined: undefined, undefined: undefined, "a": string
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:string) %12: any, type(string)
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function id(x: number): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %0: number, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %3 = CheckedTypeCastInst (:number) %2: any, type(number)
// CHECK-NEXT:       ReturnInst %3: number
// CHECK-NEXT:function_end

// CHECK:function "id 1#"(x: string): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:string) %x: string
// CHECK-NEXT:       StoreFrameInst %0: string, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %3 = CheckedTypeCastInst (:string) %2: any, type(string)
// CHECK-NEXT:       ReturnInst %3: string
// CHECK-NEXT:function_end
