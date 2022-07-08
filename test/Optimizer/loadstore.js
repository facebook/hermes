/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -strict -dump-ir %s     -O  | %FileCheck %s --match-full-lines --check-prefix=OPT-CHECK
// RUN: %hermes -hermes-parser -strict -dump-ir %s     -O0 | %FileCheck %s --match-full-lines

// Unoptimized:
//CHECK-LABEL:function foo(p1)
//CHECK-NEXT:frame = [t, k, z, y, p1]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [t]
//CHECK-NEXT:    %1 = StoreFrameInst undefined : undefined, [k]
//CHECK-NEXT:    %2 = StoreFrameInst undefined : undefined, [z]
//CHECK-NEXT:    %3 = StoreFrameInst undefined : undefined, [y]
//CHECK-NEXT:    %4 = StoreFrameInst %p1, [p1]
//CHECK-NEXT:    %5 = LoadFrameInst [p1]
//CHECK-NEXT:    %6 = StoreFrameInst %5, [t]
//CHECK-NEXT:    %7 = LoadFrameInst [t]
//CHECK-NEXT:    %8 = StoreFrameInst %7, [k]
//CHECK-NEXT:    %9 = LoadFrameInst [k]
//CHECK-NEXT:    %10 = StoreFrameInst %9, [z]
//CHECK-NEXT:    %11 = LoadFrameInst [z]
//CHECK-NEXT:    %12 = StoreFrameInst %11, [y]
//CHECK-NEXT:    %13 = LoadFrameInst [y]
//CHECK-NEXT:    %14 = ReturnInst %13
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %15 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

// Optimized:
//OPT-CHECK-LABEL:function global()
//OPT-CHECK-NEXT:frame = [], globals = [foo, test2]
//OPT-CHECK-NEXT:  %BB0:
//OPT-CHECK-NEXT:    %0 = CreateFunctionInst %foo()
//OPT-CHECK-NEXT:    %1 = StorePropertyInst %0 : closure, globalObject : object, "foo" : string
//OPT-CHECK-NEXT:    %2 = CreateFunctionInst %test2() : undefined
//OPT-CHECK-NEXT:    %3 = StorePropertyInst %2 : closure, globalObject : object, "test2" : string
//OPT-CHECK-NEXT:    %4 = LoadPropertyInst globalObject : object, "foo" : string
//OPT-CHECK-NEXT:    %5 = CallInst %4, undefined : undefined
//OPT-CHECK-NEXT:    %6 = ReturnInst %5
//OPT-CHECK-NEXT:function_end

// Optimized:
//OPT-CHECK-LABEL:function foo(p1)
//OPT-CHECK-NEXT:frame = []
//OPT-CHECK-NEXT:  %BB0:
//OPT-CHECK-NEXT:    %0 = ReturnInst %p1
//OPT-CHECK-NEXT:function_end

function foo(p1) {
  var t = p1;
  var k = t;
  var z = k;
  var y = z;
  return y;
}

foo()

//OPT-CHECK-LABEL:function test2(p1, p2) : undefined
//OPT-CHECK-NEXT:frame = []
//OPT-CHECK-NEXT:%BB0:
//OPT-CHECK-NEXT:  %0 = BinaryOperatorInst '+', %p1, %p2
//OPT-CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//OPT-CHECK-NEXT:  %2 = BinaryOperatorInst '+', %0 : string|number|bigint, 1 : number
//OPT-CHECK-NEXT:  %3 = BinaryOperatorInst '+', %0 : string|number|bigint, 2 : number
//OPT-CHECK-NEXT:  %4 = BinaryOperatorInst '+', %0 : string|number|bigint, 3 : number
//OPT-CHECK-NEXT:  %5 = BinaryOperatorInst '+', %0 : string|number|bigint, 4 : number
//OPT-CHECK-NEXT:  %6 = BinaryOperatorInst '+', %0 : string|number|bigint, 5 : number
//OPT-CHECK-NEXT:  %7 = BinaryOperatorInst '+', %0 : string|number|bigint, 6 : number
//OPT-CHECK-NEXT:  %8 = CallInst %1, undefined : undefined, %2 : string|number, %3 : string|number, %4 : string|number, %5 : string|number, %6 : string|number, %7 : string|number
//OPT-CHECK-NEXT:  %9 = ReturnInst undefined : undefined
//OPT-CHECK-NEXT:function_end
function test2(p1, p2) {
  var x = p1 + p2;
  /// No need to load X so many times.
  print(x + 1, x + 2, x + 3, x + 4, x + 5, x + 6)
}
