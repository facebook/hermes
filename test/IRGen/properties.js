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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "y": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "sink": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %sink(): any
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2: object, globalObject: object, "sink": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %5 = StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = StorePropertyLooseInst 2: number, globalObject: object, "y": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) globalObject: object, "y": string
// CHECK-NEXT:  %8 = StorePropertyLooseInst 3: number, %7: any, "bar": string
// CHECK-NEXT:  %9 = StoreStackInst 3: number, %4: any
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) globalObject: object, "y": string
// CHECK-NEXT:  %12 = CallInst (:any) %10: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %11: any
// CHECK-NEXT:  %13 = StoreStackInst %12: any, %4: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %4: any
// CHECK-NEXT:  %15 = ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function sink(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "bar": string
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "bar": string
// CHECK-NEXT:  %9 = ReturnInst %8: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %10: any, %11: any
// CHECK-NEXT:  %13 = ReturnInst %12: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
