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
// CHECK-NEXT:frame = [?anon_0_this, ?anon_1_new.target]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %this
// CHECK-NEXT:  %1 = CoerceThisNSInst %0
// CHECK-NEXT:  %2 = StoreFrameInst %1 : object, [?anon_0_this]
// CHECK-NEXT:  %3 = GetNewTargetInst %new.target
// CHECK-NEXT:  %4 = StoreFrameInst %3, [?anon_1_new.target]
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "func1" : string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "func2" : string
// CHECK-NEXT:  %7 = AllocStackInst $?anon_2_ret
// CHECK-NEXT:  %8 = StoreStackInst undefined : undefined, %7
// CHECK-NEXT:  %9 = CreateFunctionInst %func1()
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9 : closure, globalObject : object, "func1" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %func2()
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11 : closure, globalObject : object, "func2" : string
// CHECK-NEXT:  %13 = LoadStackInst %7
// CHECK-NEXT:  %14 = ReturnInst %13
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
