/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheck %s

//CHECK-LABEL:function global() : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = DeclareGlobalVarInst "main" : string
//CHECK-NEXT:  %1 = CreateFunctionInst %main() : undefined
//CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "main" : string
//CHECK-NEXT:  %3 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
//CHECK-EMPTY:
//CHECK-NEXT:function main() : undefined
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
