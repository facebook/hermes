/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function Car(model, year) {
  this.model = model;
  this.year = year;
  return "wat";
}

function test_simple_new() {
  var ctor = new Car("Eagle", 1993);
}

function test_simple_call() {
  var call = Car(1,2)
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "Car": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_simple_new": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_simple_call": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %Car(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "Car": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %test_simple_new(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "test_simple_new": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %test_simple_call(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "test_simple_call": string
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:function Car(model: any, year: any): any
// CHECK-NEXT:frame = [model: any, year: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %Car(): any, %2: environment
// CHECK-NEXT:  %4 = LoadParamInst (:any) %model: any
// CHECK-NEXT:       StoreFrameInst %3: environment, %4: any, [model]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %year: any
// CHECK-NEXT:       StoreFrameInst %3: environment, %6: any, [year]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %3: environment, [model]: any
// CHECK-NEXT:       StorePropertyLooseInst %8: any, %1: object, "model": string
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %3: environment, [year]: any
// CHECK-NEXT:        StorePropertyLooseInst %10: any, %1: object, "year": string
// CHECK-NEXT:        ReturnInst "wat": string
// CHECK-NEXT:function_end

// CHECK:function test_simple_new(): any
// CHECK-NEXT:frame = [ctor: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_simple_new(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [ctor]: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "Car": string
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: any, "prototype": string
// CHECK-NEXT:  %5 = CreateThisInst (:any) %4: any, %3: any
// CHECK-NEXT:  %6 = CallInst (:any) %3: any, empty: any, empty: any, %3: any, %5: any, "Eagle": string, 1993: number
// CHECK-NEXT:  %7 = GetConstructedObjectInst (:any) %5: any, %6: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [ctor]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_simple_call(): any
// CHECK-NEXT:frame = [call: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_simple_call(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [call]: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "Car": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number, 2: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [call]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
