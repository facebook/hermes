/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function unary_operator_test(x) {
  return +x;
  return -x;
  return ~x;
  return !x;
  return typeof x;
}

function delete_test(o) {
  delete o;
  delete o.f;
  delete o[3];
}

unary_operator_test()
delete_test()

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [unary_operator_test, delete_test]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %unary_operator_test#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "unary_operator_test" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %delete_test#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "delete_test" : string
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %6 = StoreStackInst undefined : undefined, %5
// CHECK-NEXT:  %7 = LoadPropertyInst globalObject : object, "unary_operator_test" : string
// CHECK-NEXT:  %8 = CallInst %7, undefined : undefined
// CHECK-NEXT:  %9 = StoreStackInst %8, %5
// CHECK-NEXT:  %10 = LoadPropertyInst globalObject : object, "delete_test" : string
// CHECK-NEXT:  %11 = CallInst %10, undefined : undefined
// CHECK-NEXT:  %12 = StoreStackInst %11, %5
// CHECK-NEXT:  %13 = LoadStackInst %5
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:function_end

// CHECK:function unary_operator_test#0#1(x)#2
// CHECK-NEXT:frame = [x#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{unary_operator_test#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#2], %0
// CHECK-NEXT:  %2 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %3 = AsNumberInst %2
// CHECK-NEXT:  %4 = ReturnInst %3 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %6 = UnaryOperatorInst '-', %5
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %9 = UnaryOperatorInst '~', %8
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %12 = UnaryOperatorInst '!', %11
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %15 = UnaryOperatorInst 'typeof', %14
// CHECK-NEXT:  %16 = ReturnInst %15
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function delete_test#0#1(o)#3
// CHECK-NEXT:frame = [o#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{delete_test#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %o, [o#3], %0
// CHECK-NEXT:  %2 = LoadFrameInst [o#3], %0
// CHECK-NEXT:  %3 = DeletePropertyInst %2, "f" : string
// CHECK-NEXT:  %4 = LoadFrameInst [o#3], %0
// CHECK-NEXT:  %5 = DeletePropertyInst %4, 3 : number
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
