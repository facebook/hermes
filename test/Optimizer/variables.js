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

// OPT-CHECK:function global() : undefined
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// OPT-CHECK-NEXT:  %1 = CreateFunctionInst %foo() : undefined
// OPT-CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "foo" : string
// OPT-CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// OPT-CHECK-NEXT:function_end

// OPT-CHECK:function foo(p1, p2, p3) : undefined
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = LoadParamInst %p1
// OPT-CHECK-NEXT:  %1 = LoadParamInst %p2
// OPT-CHECK-NEXT:  %2 = LoadParamInst %p3
// OPT-CHECK-NEXT:  %3 = BinaryOperatorInst '+', %0, %1
// OPT-CHECK-NEXT:  %4 = BinaryOperatorInst '+', %1, %2
// OPT-CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// OPT-CHECK-NEXT:function_end

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %foo()
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo(p1, p2, p3)
// CHECK-NEXT:frame = [p1, p2, p3, t, z, k]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %p1
// CHECK-NEXT:  %1 = StoreFrameInst %0, [p1]
// CHECK-NEXT:  %2 = LoadParamInst %p2
// CHECK-NEXT:  %3 = StoreFrameInst %2, [p2]
// CHECK-NEXT:  %4 = LoadParamInst %p3
// CHECK-NEXT:  %5 = StoreFrameInst %4, [p3]
// CHECK-NEXT:  %6 = StoreFrameInst undefined : undefined, [t]
// CHECK-NEXT:  %7 = StoreFrameInst undefined : undefined, [z]
// CHECK-NEXT:  %8 = StoreFrameInst undefined : undefined, [k]
// CHECK-NEXT:  %9 = LoadFrameInst [p1]
// CHECK-NEXT:  %10 = LoadFrameInst [p2]
// CHECK-NEXT:  %11 = BinaryOperatorInst '+', %9, %10
// CHECK-NEXT:  %12 = StoreFrameInst %11, [t]
// CHECK-NEXT:  %13 = LoadFrameInst [p2]
// CHECK-NEXT:  %14 = LoadFrameInst [p3]
// CHECK-NEXT:  %15 = BinaryOperatorInst '+', %13, %14
// CHECK-NEXT:  %16 = StoreFrameInst %15, [z]
// CHECK-NEXT:  %17 = LoadFrameInst [z]
// CHECK-NEXT:  %18 = LoadFrameInst [t]
// CHECK-NEXT:  %19 = BinaryOperatorInst '+', %17, %18
// CHECK-NEXT:  %20 = StoreFrameInst %19, [k]
// CHECK-NEXT:  %21 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %22 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
