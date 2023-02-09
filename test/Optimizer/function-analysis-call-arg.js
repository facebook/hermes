/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -fno-inline -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Function leaks as an arg to its own call.
function main() {
  var x;
  x = function f() {
    sink();
  };
  return x(x);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : string [allCallsitesKnown]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "main" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %main() : undefined
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1 : closure, globalObject : object, "main" : string
// CHECK-NEXT:  %3 = ReturnInst "use strict" : string
// CHECK-NEXT:function_end

// CHECK:function main() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %f() : undefined
// CHECK-NEXT:  %1 = CallInst %0 : closure, %f() : undefined, empty, undefined : undefined, %0 : closure
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %1 = CallInst %0, empty, empty, undefined : undefined
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
