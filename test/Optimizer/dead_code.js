/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheck %s --match-full-lines

// Make sure we can remove all trampolines from our code.

//CHECK-LABEL:function test_one(x, y, z) : string
//CHECK-NEXT:frame = []
//CHECK-NEXT:  %BB0:


function test_one(x,y,z) {
//CHECK-NEXT:    %0 = BinaryOperatorInst '+', %x, %y
  x + y

//CHECK-NEXT:    %1 = BinaryOperatorInst '+', %x, 5 : number
  x + 5

//CHECK-NEXT:    %2 = BinaryOperatorInst '+', false : boolean, %y
  false + y

//DEAD!
  8 + false

//DEAD!
  9 + "9"

//DEAD!
  8 + false

//DEAD!
  "hi" + "bye"

//Alive - result is used.
//CHECK-NEXT:    %3 = BinaryOperatorInst '+', "hi" : string, %z
  var t = "hi" + z

//DEAD!
  null + "hi"

//CHECK-NEXT:    %4 = ReturnInst %3 : string
//CHECK-NEXT:function_end
  return t
}

//CHECK-LABEL:function test_two(x, y, z) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function test_two(x,y,z) {
  function test00() {}
  var test01 = function() {}
}

