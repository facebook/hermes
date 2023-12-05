/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s     -O  | %FileCheckOrRegen %s --match-full-lines --check-prefix=OPT-CHECK
// RUN: %hermesc -hermes-parser -dump-ir %s     -O0 | %FileCheckOrRegen %s --match-full-lines

function foo(p1, p2, p3) {
  var t = p1 + p2;
  var z = p2 + p3;
  var k = z + t;
  return ;
}

// Auto-generated content below. Please do not modify manually.

// OPT-CHECK:function global(): undefined
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// OPT-CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): undefined
// OPT-CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// OPT-CHECK-NEXT:       ReturnInst undefined: undefined
// OPT-CHECK-NEXT:function_end

// OPT-CHECK:function foo(p1: any, p2: any, p3: any): undefined
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = LoadParamInst (:any) %p1: any
// OPT-CHECK-NEXT:  %1 = LoadParamInst (:any) %p2: any
// OPT-CHECK-NEXT:  %2 = LoadParamInst (:any) %p3: any
// OPT-CHECK-NEXT:  %3 = BinaryAddInst (:string|number|bigint) %0: any, %1: any
// OPT-CHECK-NEXT:  %4 = BinaryAddInst (:string|number|bigint) %1: any, %2: any
// OPT-CHECK-NEXT:  %5 = BinaryAddInst (:string|number|bigint) %4: string|number|bigint, %3: string|number|bigint
// OPT-CHECK-NEXT:       ReturnInst undefined: undefined
// OPT-CHECK-NEXT:function_end

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(p1: any, p2: any, p3: any): any
// CHECK-NEXT:frame = [p1: any, p2: any, p3: any, t: any, z: any, k: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p1: any
// CHECK-NEXT:       StoreFrameInst %0: any, [p1]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %p2: any
// CHECK-NEXT:       StoreFrameInst %2: any, [p2]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %p3: any
// CHECK-NEXT:       StoreFrameInst %4: any, [p3]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [t]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [z]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [k]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [p1]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [p2]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %9: any, %10: any
// CHECK-NEXT:        StoreFrameInst %11: any, [t]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [p2]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [p3]: any
// CHECK-NEXT:  %15 = BinaryAddInst (:any) %13: any, %14: any
// CHECK-NEXT:        StoreFrameInst %15: any, [z]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) [z]: any
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [t]: any
// CHECK-NEXT:  %19 = BinaryAddInst (:any) %17: any, %18: any
// CHECK-NEXT:        StoreFrameInst %19: any, [k]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
