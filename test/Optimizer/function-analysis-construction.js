/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -fno-inline -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Resolve construction through a variable.
function main() {
  function f() {};
  return new f();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : string [allCallsitesKnown]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "main" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %main() : object
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1 : closure, globalObject : object, "main" : string
// CHECK-NEXT:  %3 = ReturnInst "use strict" : string
// CHECK-NEXT:function_end

// CHECK:function main() : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %f() : undefined
// CHECK-NEXT:  %1 = LoadPropertyInst %0 : closure, "prototype" : string
// CHECK-NEXT:  %2 = CreateThisInst %1, %0 : closure
// CHECK-NEXT:  %3 = ConstructInst %0 : closure, %f() : undefined, empty, undefined : undefined
// CHECK-NEXT:  %4 = GetConstructedObjectInst %2 : object, %3 : undefined
// CHECK-NEXT:  %5 = ReturnInst %4 : object
// CHECK-NEXT:function_end

// CHECK:function f() : undefined [allCallsitesKnown]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
