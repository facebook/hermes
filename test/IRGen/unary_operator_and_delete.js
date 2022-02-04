/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O


//CHECK: function unary_operator_test(x)
//CHECK: frame = [x]
//CHECK:   %BB0:
//CHECK:     %1 = LoadFrameInst [x]
//CHECK:     %2 = AsNumberInst %1
//CHECK:     %3 = ReturnInst %2 : number
//CHECK:   %BB1:
//CHECK:     %4 = LoadFrameInst [x]
//CHECK:     %5 = UnaryOperatorInst '-', %4
//CHECK:     %6 = ReturnInst %5
//CHECK:   %BB2:
//CHECK:     %7 = LoadFrameInst [x]
//CHECK:     %8 = UnaryOperatorInst '~', %7
//CHECK:     %9 = ReturnInst %8
//CHECK:   %BB3:
//CHECK:     %10 = LoadFrameInst [x]
//CHECK:     %11 = UnaryOperatorInst '!', %10
//CHECK:     %12 = ReturnInst %11
//CHECK:   %BB4:
//CHECK:     %13 = LoadFrameInst [x]
//CHECK:     %14 = UnaryOperatorInst 'typeof', %13
//CHECK:     %15 = ReturnInst %14
function unary_operator_test(x) {
  return +x;
  return -x;
  return ~x;
  return !x;
  return typeof x;
}

//CHECK-LABEL:function delete_test(o)
//CHECK-NEXT:frame = [o]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %o, [o]
//CHECK-NEXT:    %1 = LoadFrameInst [o]
//CHECK-NEXT:    %2 = DeletePropertyInst %1, "f" : string
//CHECK-NEXT:    %3 = LoadFrameInst [o]
//CHECK-NEXT:    %4 = DeletePropertyInst %3, 3 : number
//CHECK-NEXT:    %5 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function delete_test(o) {
  delete o;
  delete o.f;
  delete o[3];
}

unary_operator_test()
delete_test()
