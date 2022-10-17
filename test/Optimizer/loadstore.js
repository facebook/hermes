/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -strict -dump-ir %s     -O  | %FileCheckOrRegen %s --match-full-lines --check-prefix=OPT-CHECK
// RUN: %hermes -hermes-parser -strict -dump-ir %s     -O0 | %FileCheckOrRegen %s --match-full-lines

// Unoptimized:

// Optimized:

// Optimized:

function foo(p1) {
  var t = p1;
  var k = t;
  var z = k;
  var y = z;
  return y;
}

foo()

function test2(p1, p2) {
  var x = p1 + p2;
  /// No need to load X so many times.
  print(x + 1, x + 2, x + 3, x + 4, x + 5, x + 6)
}

// Auto-generated content below. Please do not modify manually.

// OPT-CHECK:function global#0()#1
// OPT-CHECK-NEXT:frame = [], globals = [foo, test2]
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// OPT-CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// OPT-CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// OPT-CHECK-NEXT:  %3 = CreateFunctionInst %test2#0#1()#3 : undefined, %0
// OPT-CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "test2" : string
// OPT-CHECK-NEXT:  %5 = LoadPropertyInst globalObject : object, "foo" : string
// OPT-CHECK-NEXT:  %6 = CallInst %5, undefined : undefined
// OPT-CHECK-NEXT:  %7 = ReturnInst %6
// OPT-CHECK-NEXT:function_end

// OPT-CHECK:function foo#0#1(p1)#2
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// OPT-CHECK-NEXT:  %1 = ReturnInst %p1
// OPT-CHECK-NEXT:function_end

// OPT-CHECK:function test2#0#1(p1, p2)#3 : undefined
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = CreateScopeInst %S{test2#0#1()#3}
// OPT-CHECK-NEXT:  %1 = BinaryOperatorInst '+', %p1, %p2
// OPT-CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// OPT-CHECK-NEXT:  %3 = BinaryOperatorInst '+', %1 : string|number|bigint, 1 : number
// OPT-CHECK-NEXT:  %4 = BinaryOperatorInst '+', %1 : string|number|bigint, 2 : number
// OPT-CHECK-NEXT:  %5 = BinaryOperatorInst '+', %1 : string|number|bigint, 3 : number
// OPT-CHECK-NEXT:  %6 = BinaryOperatorInst '+', %1 : string|number|bigint, 4 : number
// OPT-CHECK-NEXT:  %7 = BinaryOperatorInst '+', %1 : string|number|bigint, 5 : number
// OPT-CHECK-NEXT:  %8 = BinaryOperatorInst '+', %1 : string|number|bigint, 6 : number
// OPT-CHECK-NEXT:  %9 = CallInst %2, undefined : undefined, %3 : string|number, %4 : string|number, %5 : string|number, %6 : string|number, %7 : string|number, %8 : string|number
// OPT-CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// OPT-CHECK-NEXT:function_end

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo, test2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %test2#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "test2" : string
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %6 = StoreStackInst undefined : undefined, %5
// CHECK-NEXT:  %7 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %8 = CallInst %7, undefined : undefined
// CHECK-NEXT:  %9 = StoreStackInst %8, %5
// CHECK-NEXT:  %10 = LoadStackInst %5
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(p1)#2
// CHECK-NEXT:frame = [p1#2, t#2, k#2, z#2, y#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %p1, [p1#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [t#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [k#2], %0
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [z#2], %0
// CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [y#2], %0
// CHECK-NEXT:  %6 = LoadFrameInst [p1#2], %0
// CHECK-NEXT:  %7 = StoreFrameInst %6, [t#2], %0
// CHECK-NEXT:  %8 = LoadFrameInst [t#2], %0
// CHECK-NEXT:  %9 = StoreFrameInst %8, [k#2], %0
// CHECK-NEXT:  %10 = LoadFrameInst [k#2], %0
// CHECK-NEXT:  %11 = StoreFrameInst %10, [z#2], %0
// CHECK-NEXT:  %12 = LoadFrameInst [z#2], %0
// CHECK-NEXT:  %13 = StoreFrameInst %12, [y#2], %0
// CHECK-NEXT:  %14 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test2#0#1(p1, p2)#3
// CHECK-NEXT:frame = [p1#3, p2#3, x#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test2#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %p1, [p1#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst %p2, [p2#3], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [x#3], %0
// CHECK-NEXT:  %4 = LoadFrameInst [p1#3], %0
// CHECK-NEXT:  %5 = LoadFrameInst [p2#3], %0
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %4, %5
// CHECK-NEXT:  %7 = StoreFrameInst %6, [x#3], %0
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %9 = LoadFrameInst [x#3], %0
// CHECK-NEXT:  %10 = BinaryOperatorInst '+', %9, 1 : number
// CHECK-NEXT:  %11 = LoadFrameInst [x#3], %0
// CHECK-NEXT:  %12 = BinaryOperatorInst '+', %11, 2 : number
// CHECK-NEXT:  %13 = LoadFrameInst [x#3], %0
// CHECK-NEXT:  %14 = BinaryOperatorInst '+', %13, 3 : number
// CHECK-NEXT:  %15 = LoadFrameInst [x#3], %0
// CHECK-NEXT:  %16 = BinaryOperatorInst '+', %15, 4 : number
// CHECK-NEXT:  %17 = LoadFrameInst [x#3], %0
// CHECK-NEXT:  %18 = BinaryOperatorInst '+', %17, 5 : number
// CHECK-NEXT:  %19 = LoadFrameInst [x#3], %0
// CHECK-NEXT:  %20 = BinaryOperatorInst '+', %19, 6 : number
// CHECK-NEXT:  %21 = CallInst %8, undefined : undefined, %10, %12, %14, %16, %18, %20
// CHECK-NEXT:  %22 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
