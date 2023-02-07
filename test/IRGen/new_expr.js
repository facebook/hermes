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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "Car" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "test_simple_new" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "test_simple_call" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %Car()
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3 : closure, globalObject : object, "Car" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test_simple_new()
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5 : closure, globalObject : object, "test_simple_new" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %test_simple_call()
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7 : closure, globalObject : object, "test_simple_call" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function Car(model, year)
// CHECK-NEXT:frame = [model, year]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %this
// CHECK-NEXT:  %1 = CoerceThisNSInst %0
// CHECK-NEXT:  %2 = LoadParamInst %model
// CHECK-NEXT:  %3 = StoreFrameInst %2, [model]
// CHECK-NEXT:  %4 = LoadParamInst %year
// CHECK-NEXT:  %5 = StoreFrameInst %4, [year]
// CHECK-NEXT:  %6 = LoadFrameInst [model]
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6, %1 : object, "model" : string
// CHECK-NEXT:  %8 = LoadFrameInst [year]
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8, %1 : object, "year" : string
// CHECK-NEXT:  %10 = ReturnInst "wat" : string
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_simple_new()
// CHECK-NEXT:frame = [ctor]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [ctor]
// CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "Car" : string
// CHECK-NEXT:  %2 = LoadPropertyInst %1, "prototype" : string
// CHECK-NEXT:  %3 = CreateThisInst %2, %1
// CHECK-NEXT:  %4 = ConstructInst %1, empty, empty, %3, "Eagle" : string, 1993 : number
// CHECK-NEXT:  %5 = GetConstructedObjectInst %3, %4
// CHECK-NEXT:  %6 = StoreFrameInst %5, [ctor]
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_simple_call()
// CHECK-NEXT:frame = [call]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [call]
// CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "Car" : string
// CHECK-NEXT:  %2 = CallInst %1, empty, empty, undefined : undefined, 1 : number, 2 : number
// CHECK-NEXT:  %3 = StoreFrameInst %2, [call]
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
