/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function f1(a, ...b) {}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [f1]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %f1()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "f1" : string
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function f1(a)
// CHECK-NEXT:frame = [a, b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:  %1 = CallBuiltinInst [HermesBuiltin.copyRestArgs] : number, undefined : undefined, 1 : number
// CHECK-NEXT:  %2 = StoreFrameInst %1, [b]
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
