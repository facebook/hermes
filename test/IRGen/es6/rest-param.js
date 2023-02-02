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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "f1" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %f1()
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "f1" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function f1(a)
// CHECK-NEXT:frame = [a, b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [b]
// CHECK-NEXT:  %2 = LoadParamInst %a
// CHECK-NEXT:  %3 = StoreFrameInst %2, [a]
// CHECK-NEXT:  %4 = CallBuiltinInst [HermesBuiltin.copyRestArgs] : number, undefined : undefined, 1 : number
// CHECK-NEXT:  %5 = StoreFrameInst %4, [b]
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
