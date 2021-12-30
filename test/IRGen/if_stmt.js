/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function main(boop) {
  function foo() {
    if (boop) {  } else { }
  }
}



//CHECK: function global()
//CHECK: frame = [], globals = [main]
//CHECK:   %BB0:
//CHECK:     %0 = CreateFunctionInst %main()
//CHECK:     %1 = StorePropertyInst %0 : closure, globalObject : object, "main" : string
//CHECK:     %2 = AllocStackInst $?anon_0_ret
//CHECK:     %3 = StoreStackInst undefined : undefined, %2
//CHECK:     %4 = LoadStackInst %2
//CHECK:     %5 = ReturnInst %4

//CHECK: function main(boop)
//CHECK: frame = [foo, boop]
//CHECK:   %BB0:
//CHECK:      %0 = StoreFrameInst %boop, [boop]
//CHECK:      %1 = CreateFunctionInst %foo()
//CHECK:      %2 = StoreFrameInst %1 : closure, [foo]
//CHECK:      %3 = ReturnInst undefined : undefined

//CHECK: function foo()
//CHECK: frame = []
//CHECK:   %BB0:
//CHECK:      %0 = LoadFrameInst [boop@main]
//CHECK:      %1 = CondBranchInst %0, %BB1, %BB2
//CHECK:    %BB1:
//CHECK:      %2 = BranchInst %BB3
//CHECK:    %BB2:
//CHECK:      %3 = BranchInst %BB3
//CHECK:    %BB3:
//CHECK:      %4 = ReturnInst undefined : undefined

