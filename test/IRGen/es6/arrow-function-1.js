/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

var func1 = () => 10;

var func2 = () => { return 11; }

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [?anon_0_this, ?anon_1_new.target], globals = [func1, func2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %this, [?anon_0_this]
// CHECK-NEXT:  %1 = GetNewTargetInst
// CHECK-NEXT:  %2 = StoreFrameInst %1, [?anon_1_new.target]
// CHECK-NEXT:  %3 = AllocStackInst $?anon_2_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = CreateFunctionInst %func1()
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5 : closure, globalObject : object, "func1" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %func2()
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7 : closure, globalObject : object, "func2" : string
// CHECK-NEXT:  %9 = LoadStackInst %3
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:function_end

// CHECK:arrow func1()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 10 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow func2()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 11 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
