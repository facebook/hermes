/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function sink0(a) { }
function sink1(a) { }

function test1(x,y) {
  return (1,2,3);
}

function test2(x,y) {
  return (sink0(x,y), sink1(x,y));
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [sink0, sink1, test1, test2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %sink0#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "sink0" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %sink1#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "sink1" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test1#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "test1" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %test2#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "test2" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function sink0#0#1(a)#2
// CHECK-NEXT:frame = [a#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{sink0#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#2], %0
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function sink1#0#1(a)#3
// CHECK-NEXT:frame = [a#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{sink1#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#3], %0
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test1#0#1(x, y)#4
// CHECK-NEXT:frame = [x#4, y#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test1#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#4], %0
// CHECK-NEXT:  %3 = ReturnInst 3 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test2#0#1(x, y)#5
// CHECK-NEXT:frame = [x#5, y#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test2#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#5], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#5], %0
// CHECK-NEXT:  %3 = LoadPropertyInst globalObject : object, "sink0" : string
// CHECK-NEXT:  %4 = LoadFrameInst [x#5], %0
// CHECK-NEXT:  %5 = LoadFrameInst [y#5], %0
// CHECK-NEXT:  %6 = CallInst %3, undefined : undefined, %4, %5
// CHECK-NEXT:  %7 = LoadPropertyInst globalObject : object, "sink1" : string
// CHECK-NEXT:  %8 = LoadFrameInst [x#5], %0
// CHECK-NEXT:  %9 = LoadFrameInst [y#5], %0
// CHECK-NEXT:  %10 = CallInst %7, undefined : undefined, %8, %9
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
