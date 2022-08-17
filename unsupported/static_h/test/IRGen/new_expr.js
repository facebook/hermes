/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function Car(model, year) {
  this.model = model;
  this.year = year;
  return "wat";
}


//CHECK-LABEL:function test_simple_new()
//CHECK-NEXT:frame = [ctor]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [ctor]
//CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "Car" : string
//CHECK-NEXT:  %2 = ConstructInst %1, undefined : undefined, "Eagle" : string, 1993 : number
//CHECK-NEXT:  %3 = StoreFrameInst %2 : object, [ctor]
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function test_simple_new() {
  var ctor = new Car("Eagle", 1993);
}

//CHECK-LABEL:function test_simple_call()
//CHECK-NEXT:frame = [call]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [call]
//CHECK-NEXT:    %1 = LoadPropertyInst globalObject : object, "Car" : string
//CHECK-NEXT:    %2 = CallInst %1, undefined : undefined, 1 : number, 2 : number
//CHECK-NEXT:    %3 = StoreFrameInst %2, [call]
//CHECK-NEXT:    %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function test_simple_call() {
  var call = Car(1,2)
}


