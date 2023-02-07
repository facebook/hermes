/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -strict -dump-ir %s     -O  | %FileCheckOrRegen %s --match-full-lines --check-prefix=OPT-CHECK
// RUN: %hermesc -hermes-parser -strict -dump-ir %s     -O0 | %FileCheckOrRegen %s --match-full-lines

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
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// OPT-CHECK-NEXT:  %1 = DeclareGlobalVarInst "test2" : string
// OPT-CHECK-NEXT:  %2 = CreateFunctionInst %foo()
// OPT-CHECK-NEXT:  %3 = StorePropertyStrictInst %2 : closure, globalObject : object, "foo" : string
// OPT-CHECK-NEXT:  %4 = CreateFunctionInst %test2() : undefined
// OPT-CHECK-NEXT:  %5 = StorePropertyStrictInst %4 : closure, globalObject : object, "test2" : string
// OPT-CHECK-NEXT:  %6 = LoadPropertyInst globalObject : object, "foo" : string
// OPT-CHECK-NEXT:  %7 = CallInst %6, undefined : undefined
// OPT-CHECK-NEXT:  %8 = ReturnInst %7
// OPT-CHECK-NEXT:function_end

// OPT-CHECK:function foo(p1)
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = LoadParamInst %p1
// OPT-CHECK-NEXT:  %1 = ReturnInst %0
// OPT-CHECK-NEXT:function_end

// OPT-CHECK:function test2(p1, p2) : undefined
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = LoadParamInst %p1
// OPT-CHECK-NEXT:  %1 = LoadParamInst %p2
// OPT-CHECK-NEXT:  %2 = BinaryOperatorInst '+', %0, %1
// OPT-CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// OPT-CHECK-NEXT:  %4 = BinaryOperatorInst '+', %2 : string|number|bigint, 1 : number
// OPT-CHECK-NEXT:  %5 = BinaryOperatorInst '+', %2 : string|number|bigint, 2 : number
// OPT-CHECK-NEXT:  %6 = BinaryOperatorInst '+', %2 : string|number|bigint, 3 : number
// OPT-CHECK-NEXT:  %7 = BinaryOperatorInst '+', %2 : string|number|bigint, 4 : number
// OPT-CHECK-NEXT:  %8 = BinaryOperatorInst '+', %2 : string|number|bigint, 5 : number
// OPT-CHECK-NEXT:  %9 = BinaryOperatorInst '+', %2 : string|number|bigint, 6 : number
// OPT-CHECK-NEXT:  %10 = CallInst %3, undefined : undefined, %4 : string|number, %5 : string|number, %6 : string|number, %7 : string|number, %8 : string|number, %9 : string|number
// OPT-CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// OPT-CHECK-NEXT:function_end

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "test2" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %foo()
// CHECK-NEXT:  %3 = StorePropertyStrictInst %2 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %test2()
// CHECK-NEXT:  %5 = StorePropertyStrictInst %4 : closure, globalObject : object, "test2" : string
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %7 = StoreStackInst undefined : undefined, %6
// CHECK-NEXT:  %8 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %9 = CallInst %8, undefined : undefined
// CHECK-NEXT:  %10 = StoreStackInst %9, %6
// CHECK-NEXT:  %11 = LoadStackInst %6
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function foo(p1)
// CHECK-NEXT:frame = [p1, t, k, z, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %p1
// CHECK-NEXT:  %1 = StoreFrameInst %0, [p1]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [t]
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [k]
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [z]
// CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [y]
// CHECK-NEXT:  %6 = LoadFrameInst [p1]
// CHECK-NEXT:  %7 = StoreFrameInst %6, [t]
// CHECK-NEXT:  %8 = LoadFrameInst [t]
// CHECK-NEXT:  %9 = StoreFrameInst %8, [k]
// CHECK-NEXT:  %10 = LoadFrameInst [k]
// CHECK-NEXT:  %11 = StoreFrameInst %10, [z]
// CHECK-NEXT:  %12 = LoadFrameInst [z]
// CHECK-NEXT:  %13 = StoreFrameInst %12, [y]
// CHECK-NEXT:  %14 = LoadFrameInst [y]
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test2(p1, p2)
// CHECK-NEXT:frame = [p1, p2, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %p1
// CHECK-NEXT:  %1 = StoreFrameInst %0, [p1]
// CHECK-NEXT:  %2 = LoadParamInst %p2
// CHECK-NEXT:  %3 = StoreFrameInst %2, [p2]
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %5 = LoadFrameInst [p1]
// CHECK-NEXT:  %6 = LoadFrameInst [p2]
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %5, %6
// CHECK-NEXT:  %8 = StoreFrameInst %7, [x]
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %10 = LoadFrameInst [x]
// CHECK-NEXT:  %11 = BinaryOperatorInst '+', %10, 1 : number
// CHECK-NEXT:  %12 = LoadFrameInst [x]
// CHECK-NEXT:  %13 = BinaryOperatorInst '+', %12, 2 : number
// CHECK-NEXT:  %14 = LoadFrameInst [x]
// CHECK-NEXT:  %15 = BinaryOperatorInst '+', %14, 3 : number
// CHECK-NEXT:  %16 = LoadFrameInst [x]
// CHECK-NEXT:  %17 = BinaryOperatorInst '+', %16, 4 : number
// CHECK-NEXT:  %18 = LoadFrameInst [x]
// CHECK-NEXT:  %19 = BinaryOperatorInst '+', %18, 5 : number
// CHECK-NEXT:  %20 = LoadFrameInst [x]
// CHECK-NEXT:  %21 = BinaryOperatorInst '+', %20, 6 : number
// CHECK-NEXT:  %22 = CallInst %9, undefined : undefined, %11, %13, %15, %17, %19, %21
// CHECK-NEXT:  %23 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
