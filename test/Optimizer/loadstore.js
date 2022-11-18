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

// OPT-CHECK:function global()
// OPT-CHECK-NEXT:frame = [], globals = [foo, test2]
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = CreateFunctionInst %foo()
// OPT-CHECK-NEXT:  %1 = StorePropertyStrictInst %0 : closure, globalObject : object, "foo" : string
// OPT-CHECK-NEXT:  %2 = CreateFunctionInst %test2() : undefined
// OPT-CHECK-NEXT:  %3 = StorePropertyStrictInst %2 : closure, globalObject : object, "test2" : string
// OPT-CHECK-NEXT:  %4 = LoadPropertyInst globalObject : object, "foo" : string
// OPT-CHECK-NEXT:  %5 = CallInst %4, undefined : undefined
// OPT-CHECK-NEXT:  %6 = ReturnInst %5
// OPT-CHECK-NEXT:function_end

// OPT-CHECK:function foo(p1)
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = ReturnInst %p1
// OPT-CHECK-NEXT:function_end

// OPT-CHECK:function test2(p1, p2) : undefined
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = BinaryOperatorInst '+', %p1, %p2
// OPT-CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// OPT-CHECK-NEXT:  %2 = BinaryOperatorInst '+', %0 : string|number|bigint, 1 : number
// OPT-CHECK-NEXT:  %3 = BinaryOperatorInst '+', %0 : string|number|bigint, 2 : number
// OPT-CHECK-NEXT:  %4 = BinaryOperatorInst '+', %0 : string|number|bigint, 3 : number
// OPT-CHECK-NEXT:  %5 = BinaryOperatorInst '+', %0 : string|number|bigint, 4 : number
// OPT-CHECK-NEXT:  %6 = BinaryOperatorInst '+', %0 : string|number|bigint, 5 : number
// OPT-CHECK-NEXT:  %7 = BinaryOperatorInst '+', %0 : string|number|bigint, 6 : number
// OPT-CHECK-NEXT:  %8 = CallInst %1, undefined : undefined, %2 : string|number, %3 : string|number, %4 : string|number, %5 : string|number, %6 : string|number, %7 : string|number
// OPT-CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// OPT-CHECK-NEXT:function_end

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [foo, test2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo()
// CHECK-NEXT:  %1 = StorePropertyStrictInst %0 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %test2()
// CHECK-NEXT:  %3 = StorePropertyStrictInst %2 : closure, globalObject : object, "test2" : string
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %5 = StoreStackInst undefined : undefined, %4
// CHECK-NEXT:  %6 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %7 = CallInst %6, undefined : undefined
// CHECK-NEXT:  %8 = StoreStackInst %7, %4
// CHECK-NEXT:  %9 = LoadStackInst %4
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:function_end

// CHECK:function foo(p1)
// CHECK-NEXT:frame = [t, k, z, y, p1]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [t]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [k]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [z]
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [y]
// CHECK-NEXT:  %4 = StoreFrameInst %p1, [p1]
// CHECK-NEXT:  %5 = LoadFrameInst [p1]
// CHECK-NEXT:  %6 = StoreFrameInst %5, [t]
// CHECK-NEXT:  %7 = LoadFrameInst [t]
// CHECK-NEXT:  %8 = StoreFrameInst %7, [k]
// CHECK-NEXT:  %9 = LoadFrameInst [k]
// CHECK-NEXT:  %10 = StoreFrameInst %9, [z]
// CHECK-NEXT:  %11 = LoadFrameInst [z]
// CHECK-NEXT:  %12 = StoreFrameInst %11, [y]
// CHECK-NEXT:  %13 = LoadFrameInst [y]
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %15 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test2(p1, p2)
// CHECK-NEXT:frame = [x, p1, p2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %p1, [p1]
// CHECK-NEXT:  %2 = StoreFrameInst %p2, [p2]
// CHECK-NEXT:  %3 = LoadFrameInst [p1]
// CHECK-NEXT:  %4 = LoadFrameInst [p2]
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %3, %4
// CHECK-NEXT:  %6 = StoreFrameInst %5, [x]
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %8 = LoadFrameInst [x]
// CHECK-NEXT:  %9 = BinaryOperatorInst '+', %8, 1 : number
// CHECK-NEXT:  %10 = LoadFrameInst [x]
// CHECK-NEXT:  %11 = BinaryOperatorInst '+', %10, 2 : number
// CHECK-NEXT:  %12 = LoadFrameInst [x]
// CHECK-NEXT:  %13 = BinaryOperatorInst '+', %12, 3 : number
// CHECK-NEXT:  %14 = LoadFrameInst [x]
// CHECK-NEXT:  %15 = BinaryOperatorInst '+', %14, 4 : number
// CHECK-NEXT:  %16 = LoadFrameInst [x]
// CHECK-NEXT:  %17 = BinaryOperatorInst '+', %16, 5 : number
// CHECK-NEXT:  %18 = LoadFrameInst [x]
// CHECK-NEXT:  %19 = BinaryOperatorInst '+', %18, 6 : number
// CHECK-NEXT:  %20 = CallInst %7, undefined : undefined, %9, %11, %13, %15, %17, %19
// CHECK-NEXT:  %21 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
