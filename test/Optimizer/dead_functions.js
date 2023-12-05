/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s     -O | %FileCheck %s

//CHECK-LABEL:function global(): undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:       DeclareGlobalVarInst "main": string
//CHECK-NEXT:  %1 = CreateFunctionInst (:object) %main(): undefined
//CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "main": string
//CHECK-NEXT:       ReturnInst undefined: undefined
//CHECK-NEXT:function_end
//CHECK-EMPTY:
//CHECK-NEXT:function main(): undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:       ReturnInst undefined: undefined
//CHECK-NEXT:function_end

// No more functions in this module.
//CHECK-NOT: function

function main() {
  var k = "captured";

  // DEC needs to remove these functions:
  var x0 = function () { return "hi" }
  var x1 = function () { return function () { return "nested" + k } }
  var x2 = function () { return 1 + 2 }

  function f1() {
    function f2() {
      return function f3() {
        f2();
      };
    }
  }
}
