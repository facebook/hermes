/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

var y = 2;
y.bar = 3;
sink(y)

function sink(x, y) {
  return x.bar
  return x["bar"]
  return x[y]
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [y, sink]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %sink#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = StorePropertyInst 2 : number, globalObject : object, "y" : string
// CHECK-NEXT:  %6 = LoadPropertyInst globalObject : object, "y" : string
// CHECK-NEXT:  %7 = StorePropertyInst 3 : number, %6, "bar" : string
// CHECK-NEXT:  %8 = StoreStackInst 3 : number, %3
// CHECK-NEXT:  %9 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %10 = LoadPropertyInst globalObject : object, "y" : string
// CHECK-NEXT:  %11 = CallInst %9, undefined : undefined, %10
// CHECK-NEXT:  %12 = StoreStackInst %11, %3
// CHECK-NEXT:  %13 = LoadStackInst %3
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:function_end

// CHECK:function sink#0#1(x, y)#2
// CHECK-NEXT:frame = [x#2, y#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{sink#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#2], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %4 = LoadPropertyInst %3, "bar" : string
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %7 = LoadPropertyInst %6, "bar" : string
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %10 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %11 = LoadPropertyInst %9, %10
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
