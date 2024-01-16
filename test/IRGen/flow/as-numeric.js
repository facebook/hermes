/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function main() {
  function baz(x: number): void {
    x++;
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
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(): any
// CHECK-NEXT:frame = [baz: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %baz(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: object, [baz]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function baz(x: number): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %0: number, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %3 = AsNumericInst (:number|bigint) %2: any
// CHECK-NEXT:  %4 = UnaryIncInst (:any) %3: number|bigint
// CHECK-NEXT:       StoreFrameInst %4: any, [x]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
