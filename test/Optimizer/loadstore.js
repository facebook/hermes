/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -strict -dump-ir %s     -O  | %FileCheckOrRegen %s --match-full-lines --check-prefix=OPT-CHECK
// RUN: %hermesc -hermes-parser -strict -dump-ir %s     -O0 | %FileCheckOrRegen %s --match-full-lines

// Unoptimized:

// Optimized:

// Optimized:

function foo(p1) {
  var t = p1;
  var k = t;
  var z = k;
  var y = z;
  return y;
}

foo()

function test2(p1, p2) {
  var x = p1 + p2;
  /// No need to load X so many times.
  print(x + 1, x + 2, x + 3, x + 4, x + 5, x + 6)
}

// Auto-generated content below. Please do not modify manually.

// OPT-CHECK:function global(): any
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// OPT-CHECK-NEXT:  %1 = DeclareGlobalVarInst "test2": string
// OPT-CHECK-NEXT:  %2 = CreateFunctionInst (:object) %foo(): any
// OPT-CHECK-NEXT:  %3 = StorePropertyStrictInst %2: object, globalObject: object, "foo": string
// OPT-CHECK-NEXT:  %4 = CreateFunctionInst (:object) %test2(): undefined
// OPT-CHECK-NEXT:  %5 = StorePropertyStrictInst %4: object, globalObject: object, "test2": string
// OPT-CHECK-NEXT:  %6 = LoadPropertyInst (:any) globalObject: object, "foo": string
// OPT-CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// OPT-CHECK-NEXT:  %8 = ReturnInst %7: any
// OPT-CHECK-NEXT:function_end

// OPT-CHECK:function foo(p1: any): any
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = LoadParamInst (:any) %p1: any
// OPT-CHECK-NEXT:  %1 = ReturnInst %0: any
// OPT-CHECK-NEXT:function_end

// OPT-CHECK:function test2(p1: any, p2: any): undefined
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = LoadParamInst (:any) %p1: any
// OPT-CHECK-NEXT:  %1 = LoadParamInst (:any) %p2: any
// OPT-CHECK-NEXT:  %2 = BinaryAddInst (:string|number|bigint) %0: any, %1: any
// OPT-CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-CHECK-NEXT:  %4 = BinaryAddInst (:string|number) %2: string|number|bigint, 1: number
// OPT-CHECK-NEXT:  %5 = BinaryAddInst (:string|number) %2: string|number|bigint, 2: number
// OPT-CHECK-NEXT:  %6 = BinaryAddInst (:string|number) %2: string|number|bigint, 3: number
// OPT-CHECK-NEXT:  %7 = BinaryAddInst (:string|number) %2: string|number|bigint, 4: number
// OPT-CHECK-NEXT:  %8 = BinaryAddInst (:string|number) %2: string|number|bigint, 5: number
// OPT-CHECK-NEXT:  %9 = BinaryAddInst (:string|number) %2: string|number|bigint, 6: number
// OPT-CHECK-NEXT:  %10 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: string|number, %5: string|number, %6: string|number, %7: string|number, %8: string|number, %9: string|number
// OPT-CHECK-NEXT:  %11 = ReturnInst undefined: undefined
// OPT-CHECK-NEXT:function_end

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "test2": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:  %3 = StorePropertyStrictInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %test2(): any
// CHECK-NEXT:  %5 = StorePropertyStrictInst %4: object, globalObject: object, "test2": string
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %7 = StoreStackInst undefined: undefined, %6: any
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %9 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %10 = StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %12 = ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:function foo(p1: any): any
// CHECK-NEXT:frame = [p1: any, t: any, k: any, z: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p1: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [p1]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [t]: any
// CHECK-NEXT:  %3 = StoreFrameInst undefined: undefined, [k]: any
// CHECK-NEXT:  %4 = StoreFrameInst undefined: undefined, [z]: any
// CHECK-NEXT:  %5 = StoreFrameInst undefined: undefined, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [p1]: any
// CHECK-NEXT:  %7 = StoreFrameInst %6: any, [t]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [t]: any
// CHECK-NEXT:  %9 = StoreFrameInst %8: any, [k]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [k]: any
// CHECK-NEXT:  %11 = StoreFrameInst %10: any, [z]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [z]: any
// CHECK-NEXT:  %13 = StoreFrameInst %12: any, [y]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %15 = ReturnInst %14: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test2(p1: any, p2: any): any
// CHECK-NEXT:frame = [p1: any, p2: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p1: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [p1]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %p2: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [p2]: any
// CHECK-NEXT:  %4 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [p1]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [p2]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %5: any, %6: any
// CHECK-NEXT:  %8 = StoreFrameInst %7: any, [x]: any
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %10: any, 1: number
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %13 = BinaryAddInst (:any) %12: any, 2: number
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %15 = BinaryAddInst (:any) %14: any, 3: number
// CHECK-NEXT:  %16 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %17 = BinaryAddInst (:any) %16: any, 4: number
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %19 = BinaryAddInst (:any) %18: any, 5: number
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %21 = BinaryAddInst (:any) %20: any, 6: number
// CHECK-NEXT:  %22 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %11: any, %13: any, %15: any, %17: any, %19: any, %21: any
// CHECK-NEXT:  %23 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
