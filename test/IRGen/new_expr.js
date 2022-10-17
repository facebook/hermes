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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [Car, test_simple_new, test_simple_call]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %Car#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "Car" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %test_simple_new#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "test_simple_new" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test_simple_call#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "test_simple_call" : string
// CHECK-NEXT:  %7 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %8 = StoreStackInst undefined : undefined, %7
// CHECK-NEXT:  %9 = LoadStackInst %7
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:function_end

// CHECK:function Car#0#1(model, year)#2
// CHECK-NEXT:frame = [model#2, year#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{Car#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %model, [model#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %year, [year#2], %0
// CHECK-NEXT:  %3 = LoadFrameInst [model#2], %0
// CHECK-NEXT:  %4 = StorePropertyInst %3, %this, "model" : string
// CHECK-NEXT:  %5 = LoadFrameInst [year#2], %0
// CHECK-NEXT:  %6 = StorePropertyInst %5, %this, "year" : string
// CHECK-NEXT:  %7 = ReturnInst "wat" : string
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_simple_new#0#1()#3
// CHECK-NEXT:frame = [ctor#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_simple_new#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [ctor#3], %0
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "Car" : string
// CHECK-NEXT:  %3 = ConstructInst %2, undefined : undefined, "Eagle" : string, 1993 : number
// CHECK-NEXT:  %4 = StoreFrameInst %3 : object, [ctor#3], %0
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_simple_call#0#1()#4
// CHECK-NEXT:frame = [call#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_simple_call#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [call#4], %0
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "Car" : string
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined, 1 : number, 2 : number
// CHECK-NEXT:  %4 = StoreFrameInst %3, [call#4], %0
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
