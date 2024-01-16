/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function sink0(a) { }
function sink1(a) { }

function test1(x,y) {
  return (1,2,3);
}

function test2(x,y) {
  return (sink0(x,y), sink1(x,y));
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "sink0": string
// CHECK-NEXT:       DeclareGlobalVarInst "sink1": string
// CHECK-NEXT:       DeclareGlobalVarInst "test1": string
// CHECK-NEXT:       DeclareGlobalVarInst "test2": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %sink0(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "sink0": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %sink1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "sink1": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %test1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "test1": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %test2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "test2": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function sink0(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function sink1(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test1(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:       ReturnInst 3: number
// CHECK-NEXT:function_end

// CHECK:function test2(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) globalObject: object, "sink0": string
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %7 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %5: any, %6: any
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) globalObject: object, "sink1": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %11 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %9: any, %10: any
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end
