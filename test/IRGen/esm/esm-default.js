/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -commonjs -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

export default function() {
  return 400;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0
// CHECK-NEXT:  %2 = LoadStackInst %0
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:function_end

// CHECK:function cjs_module(exports, require, module)
// CHECK-NEXT:frame = [exports, require, module]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %exports, [exports]
// CHECK-NEXT:  %1 = StoreFrameInst %require, [require]
// CHECK-NEXT:  %2 = StoreFrameInst %module, [module]
// CHECK-NEXT:  %3 = CreateFunctionInst %""()
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3 : closure, %exports, "?default" : string
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function ""()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 400 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
