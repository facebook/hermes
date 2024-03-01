/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

var y = 2;
y.bar = 3;
sink(y)

function sink(x, y) {
  return x.bar
  return x["bar"]
  return x[y]
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "y": string
// CHECK-NEXT:       DeclareGlobalVarInst "sink": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %sink(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "sink": string
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %5: any
// CHECK-NEXT:       StorePropertyLooseInst 2: number, globalObject: object, "y": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) globalObject: object, "y": string
// CHECK-NEXT:       StorePropertyLooseInst 3: number, %8: any, "bar": string
// CHECK-NEXT:        StoreStackInst 3: number, %5: any
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) globalObject: object, "y": string
// CHECK-NEXT:  %13 = CallInst (:any) %11: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %12: any
// CHECK-NEXT:        StoreStackInst %13: any, %5: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %5: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function sink(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %sink(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "bar": string
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end
