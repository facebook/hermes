/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

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
// CHECK-NEXT:frame = [], globals = [Car, test_simple_new, test_simple_call]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %Car()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "Car" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %test_simple_new()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "test_simple_new" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %test_simple_call()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "test_simple_call" : string
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %7 = StoreStackInst undefined : undefined, %6
// CHECK-NEXT:  %8 = LoadStackInst %6
// CHECK-NEXT:  %9 = ReturnInst %8
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
// CHECK-NEXT:  %3 = HBCCreateThisInst %2, %1
// CHECK-NEXT:  %4 = HBCConstructInst %1, %3, "Eagle" : string, 1993 : number
// CHECK-NEXT:  %5 = HBCGetConstructedObjectInst %3, %4
// CHECK-NEXT:  %6 = StoreFrameInst %5, [ctor]
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_simple_call()
// CHECK-NEXT:frame = [call]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [call]
// CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "Car" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 1 : number, 2 : number
// CHECK-NEXT:  %3 = StoreFrameInst %2, [call]
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
