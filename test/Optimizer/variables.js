/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O  | %FileCheckOrRegen %s --match-full-lines --check-prefix=OPT-CHECK
// RUN: %hermes -hermes-parser -dump-ir %s     -O0 | %FileCheckOrRegen %s --match-full-lines

function foo(p1, p2, p3) {
  var t = p1 + p2;
  var z = p2 + p3;
  var k = z + t;
  return ;
}

// Auto-generated content below. Please do not modify manually.

// OPT-CHECK:function global#0()#1 : undefined
// OPT-CHECK-NEXT:frame = [], globals = [foo]
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// OPT-CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2 : undefined, %0
// OPT-CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// OPT-CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// OPT-CHECK-NEXT:function_end

// OPT-CHECK:function foo#0#1(p1, p2, p3)#2 : undefined
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// OPT-CHECK-NEXT:  %1 = BinaryOperatorInst '+', %p1, %p2
// OPT-CHECK-NEXT:  %2 = BinaryOperatorInst '+', %p2, %p3
// OPT-CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// OPT-CHECK-NEXT:function_end

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(p1, p2, p3)#2
// CHECK-NEXT:frame = [p1#2, p2#2, p3#2, t#2, z#2, k#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %p1, [p1#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %p2, [p2#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst %p3, [p3#2], %0
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [t#2], %0
// CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [z#2], %0
// CHECK-NEXT:  %6 = StoreFrameInst undefined : undefined, [k#2], %0
// CHECK-NEXT:  %7 = LoadFrameInst [p1#2], %0
// CHECK-NEXT:  %8 = LoadFrameInst [p2#2], %0
// CHECK-NEXT:  %9 = BinaryOperatorInst '+', %7, %8
// CHECK-NEXT:  %10 = StoreFrameInst %9, [t#2], %0
// CHECK-NEXT:  %11 = LoadFrameInst [p2#2], %0
// CHECK-NEXT:  %12 = LoadFrameInst [p3#2], %0
// CHECK-NEXT:  %13 = BinaryOperatorInst '+', %11, %12
// CHECK-NEXT:  %14 = StoreFrameInst %13, [z#2], %0
// CHECK-NEXT:  %15 = LoadFrameInst [z#2], %0
// CHECK-NEXT:  %16 = LoadFrameInst [t#2], %0
// CHECK-NEXT:  %17 = BinaryOperatorInst '+', %15, %16
// CHECK-NEXT:  %18 = StoreFrameInst %17, [k#2], %0
// CHECK-NEXT:  %19 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %20 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
