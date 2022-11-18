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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [sink0, sink1, test1, test2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %sink0()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "sink0" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %sink1()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "sink1" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %test1()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "test1" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %test2()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "test2" : string
// CHECK-NEXT:  %8 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %9 = StoreStackInst undefined : undefined, %8
// CHECK-NEXT:  %10 = LoadStackInst %8
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:function sink0(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function sink1(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test1(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %2 = ReturnInst 3 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test2(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "sink0" : string
// CHECK-NEXT:  %3 = LoadFrameInst [x]
// CHECK-NEXT:  %4 = LoadFrameInst [y]
// CHECK-NEXT:  %5 = CallInst %2, undefined : undefined, %3, %4
// CHECK-NEXT:  %6 = LoadPropertyInst globalObject : object, "sink1" : string
// CHECK-NEXT:  %7 = LoadFrameInst [x]
// CHECK-NEXT:  %8 = LoadFrameInst [y]
// CHECK-NEXT:  %9 = CallInst %6, undefined : undefined, %7, %8
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
