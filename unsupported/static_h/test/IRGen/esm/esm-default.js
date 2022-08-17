/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -commonjs -dump-ir %s | %FileCheck --match-full-lines %s

export default function() {
  return 400;
}

// CHECK-LABEL: function cjs_module(exports, require, module)
// CHECK-NEXT: frame = [exports, require, module]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %exports, [exports]
// CHECK-NEXT:   %1 = StoreFrameInst %require, [require]
// CHECK-NEXT:   %2 = StoreFrameInst %module, [module]
// CHECK-NEXT:   %3 = CreateFunctionInst %""()
// CHECK-NEXT:   %4 = StorePropertyInst %3 : closure, %exports, "?default" : string
// CHECK-NEXT:   %5 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

// CHECK-LABEL: function ""()
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = ReturnInst 400 : number
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %1 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end
