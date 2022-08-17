/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheck %s --match-full-lines


//CHECK-LABEL:function foo() : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function foo () {
  return;
  var a;
  for(;;)
    ++a;
}

//CHECK-LABEL:function bar() : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryStartInst %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %1 = CatchInst
//CHECK-NEXT:  %2 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %3 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %4 = TryEndInst
//CHECK-NEXT:  %5 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function bar() {
  // This will lead to unreachable cyclic blocks covered by catch
  var i = 0;
  try {
    return;
    for(;;)
      ++i;
  } catch (e) {
  }
}

foo();
bar();
