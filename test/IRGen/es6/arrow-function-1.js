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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [?anon_0_this#1, ?anon_1_new.target#1], globals = [func1, func2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = StoreFrameInst %this, [?anon_0_this#1], %0
// CHECK-NEXT:  %2 = GetNewTargetInst
// CHECK-NEXT:  %3 = StoreFrameInst %2, [?anon_1_new.target#1], %0
// CHECK-NEXT:  %4 = AllocStackInst $?anon_2_ret
// CHECK-NEXT:  %5 = StoreStackInst undefined : undefined, %4
// CHECK-NEXT:  %6 = CreateFunctionInst %func1#0#1()#2, %0
// CHECK-NEXT:  %7 = StorePropertyInst %6 : closure, globalObject : object, "func1" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %func2#0#1()#3, %0
// CHECK-NEXT:  %9 = StorePropertyInst %8 : closure, globalObject : object, "func2" : string
// CHECK-NEXT:  %10 = LoadStackInst %4
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:arrow func1#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{func1#0#1()#2}
// CHECK-NEXT:  %1 = ReturnInst 10 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow func2#0#1()#3
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{func2#0#1()#3}
// CHECK-NEXT:  %1 = ReturnInst 11 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
