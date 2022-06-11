/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fn_dir/run_fnc.sh %fnc %s %t && %t | %FileCheck %s --match-full-lines

function assert(pred){
  if(pred != true){
    throw "Assertion failed";
  }
}

var x = [5, undefined, null, "foo", {}, function(){}]

for(var i = 0; i < 6; i++){
  for(var j = 0; j < 6; j++){
    print(x[i]);
    if(x[i] == x[j]){
      print(" == ");
    } else {
      assert(x[i] != x[j]);
      print(" != ");
    }
    print(x[j]);
    print("\n")
  }
}
// CHECK: 5.000000 != null
// CHECK-NEXT: 5.000000 != foo
// CHECK-NEXT: 5.000000 != [Object]
// CHECK-NEXT: 5.000000 != [Closure]
// CHECK-NEXT: undefined != 5.000000
// CHECK-NEXT: undefined == undefined
// CHECK-NEXT: undefined != null
// CHECK-NEXT: undefined != foo
// CHECK-NEXT: undefined != [Object]
// CHECK-NEXT: undefined != [Closure]
// CHECK-NEXT: null != 5.000000
// CHECK-NEXT: null != undefined
// CHECK-NEXT: null == null
// CHECK-NEXT: null != foo
// CHECK-NEXT: null != [Object]
// CHECK-NEXT: null != [Closure]
// CHECK-NEXT: foo != 5.000000
// CHECK-NEXT: foo != undefined
// CHECK-NEXT: foo != null
// CHECK-NEXT: foo == foo
// CHECK-NEXT: foo != [Object]
// CHECK-NEXT: foo != [Closure]
// CHECK-NEXT: [Object] != 5.000000
// CHECK-NEXT: [Object] != undefined
// CHECK-NEXT: [Object] != null
// CHECK-NEXT: [Object] != foo
// CHECK-NEXT: [Object] == [Object]
// CHECK-NEXT: [Object] != [Closure]
// CHECK-NEXT: [Closure] != 5.000000
// CHECK-NEXT: [Closure] != undefined
// CHECK-NEXT: [Closure] != null
// CHECK-NEXT: [Closure] != foo
// CHECK-NEXT: [Closure] != [Object]
// CHECK-NEXT: [Closure] == [Closure]
