/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -O | %FileCheck --match-full-lines %s

//CHECK-LABEL:function bar() : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = LoadFrameInst [p@main]
//CHECK-NEXT:  %1 = LoadFrameInst [k@main]
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, %1
//CHECK-NEXT:  %3 = LoadFrameInst [p@main]
//CHECK-NEXT:  %4 = CallInst %3, undefined : undefined, %1
//CHECK-NEXT:  %5 = LoadFrameInst [p@main]
//CHECK-NEXT:  %6 = CallInst %5, undefined : undefined, %1
//CHECK-NEXT:  %7 = LoadFrameInst [p@main]
//CHECK-NEXT:  %8 = CallInst %7, undefined : undefined, %1
//CHECK-NEXT:  %9 = LoadFrameInst [p@main]
//CHECK-NEXT:  %10 = CallInst %9, undefined : undefined, %1
//CHECK-NEXT:  %11 = LoadFrameInst [p@main]
//CHECK-NEXT:  %12 = CallInst %11, undefined : undefined, %1
//CHECK-NEXT:  %13 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function main(p) {
  var k = p;
  var p = print;
  function bar() {
    p(k)
    p(k)
    p(k)
    p(k)
    p(k)
    p(k)
  }

  return bar;
}
