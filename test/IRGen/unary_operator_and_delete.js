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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [unary_operator_test, delete_test]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %unary_operator_test()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "unary_operator_test" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %delete_test()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "delete_test" : string
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %5 = StoreStackInst undefined : undefined, %4
// CHECK-NEXT:  %6 = LoadPropertyInst globalObject : object, "unary_operator_test" : string
// CHECK-NEXT:  %7 = CallInst %6, undefined : undefined
// CHECK-NEXT:  %8 = StoreStackInst %7, %4
// CHECK-NEXT:  %9 = LoadPropertyInst globalObject : object, "delete_test" : string
// CHECK-NEXT:  %10 = CallInst %9, undefined : undefined
// CHECK-NEXT:  %11 = StoreStackInst %10, %4
// CHECK-NEXT:  %12 = LoadStackInst %4
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:function_end

// CHECK:function unary_operator_test(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = LoadFrameInst [x]
// CHECK-NEXT:  %2 = AsNumberInst %1
// CHECK-NEXT:  %3 = ReturnInst %2 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = UnaryOperatorInst '-', %4
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst [x]
// CHECK-NEXT:  %8 = UnaryOperatorInst '~', %7
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = LoadFrameInst [x]
// CHECK-NEXT:  %11 = UnaryOperatorInst '!', %10
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = LoadFrameInst [x]
// CHECK-NEXT:  %14 = UnaryOperatorInst 'typeof', %13
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %16 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function delete_test(o)
// CHECK-NEXT:frame = [o]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %o, [o]
// CHECK-NEXT:  %1 = LoadFrameInst [o]
// CHECK-NEXT:  %2 = DeletePropertyInst %1, "f" : string
// CHECK-NEXT:  %3 = LoadFrameInst [o]
// CHECK-NEXT:  %4 = DeletePropertyInst %3, 3 : number
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
