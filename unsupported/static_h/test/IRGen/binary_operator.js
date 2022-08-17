/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

  function foo(x, y) {
    x = y;
    x += y;
    x -= y;
    x *= y;
    x /= y;
    x %= y;
    //x **= y; Not ES5.
    x <<= y;
    x >>= y;
    x >>>= y;
    x &= y;
    x ^= y;
    x |= y;


    x.t = y;
    x.t += y;
    x.t -= y;
    x.t *= y;
    x.t /= y;
    x.t %= y;
    //x.t **= y; Not ES5.
    x.t <<= y;
    x.t >>= y;
    x.t >>>= y;
    x.t &= y;
    x.t ^= y;
    x.t |= y;


    return x == y;
    return x != y;
    return x === y;
    // return x !=== y; Not ES5.
    return x < y;
    return x <= y;
    return x > y;
    return x >= y;
    return x << y;
    return x << y;
    return x >>> y;
    return x + y;
    return x - y;
    return x * y;
    return x / y;
    return x % y;
    return x | y;
    return x ^ y;
    return x & y;
    return x in y;
    return x instanceof y;
  }


//CHECK-LABEL:function assignment_test(x, y)
//CHECK-NEXT:frame = [x, y]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %x, [x]
//CHECK-NEXT:    %1 = StoreFrameInst %y, [y]
//CHECK-NEXT:    %2 = LoadFrameInst [y]
//CHECK-NEXT:    %3 = StoreFrameInst %2, [x]
//CHECK-NEXT:    %4 = LoadFrameInst [x]
//CHECK-NEXT:    %5 = LoadFrameInst [y]
//CHECK-NEXT:    %6 = BinaryOperatorInst '+', %4, %5
//CHECK-NEXT:    %7 = StoreFrameInst %6, [x]
//CHECK-NEXT:    %8 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
  function assignment_test(x, y) {
    x = y;
    x += y;
  }

//CHECK-LABEL:function member_test(x, y)
//CHECK-NEXT:frame = [x, y]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %x, [x]
//CHECK-NEXT:    %1 = StoreFrameInst %y, [y]
//CHECK-NEXT:    %2 = LoadFrameInst [x]
//CHECK-NEXT:    %3 = LoadPropertyInst %2, "t" : string
//CHECK-NEXT:    %4 = LoadFrameInst [y]
//CHECK-NEXT:    %5 = BinaryOperatorInst '+', %3, %4
//CHECK-NEXT:    %6 = StorePropertyInst %5, %2, "t" : string
//CHECK-NEXT:    %7 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
  function member_test(x, y) {
    x.t += y;
  }

//CHECK: function binary_ops(x, y)
  function binary_ops(x, y) {
//CHECK:     %2 = LoadFrameInst [x]
//CHECK:     %3 = LoadFrameInst [y]
//CHECK:     %4 = BinaryOperatorInst '>>>', %2, %3
//CHECK:     %5 = ReturnInst %4
    return x >>> y;
  }

