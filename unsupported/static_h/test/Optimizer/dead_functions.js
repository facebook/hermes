/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheck %s


//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [], globals = [main]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateFunctionInst %main()
//CHECK-NEXT:  %1 = StorePropertyInst %0 : closure, globalObject : object, "main" : string
//CHECK-NEXT:  %2 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHECK-LABEL:function main()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

// No more functions in this module.
//CHECK-NOT: function

function main() {
  var k = "captured";

  // DEC needs to remove these functions:
  var x0 = function () { return "hi" }
  var x1 = function () { return function () { return "nested" + k } }
  var x2 = function () { return 1 + 2 }

}
