/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function main(boop) {
  function foo() {
    if (boop) {  } else { }
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [main]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %main()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "main" : string
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function main(boop)
// CHECK-NEXT:frame = [foo, boop]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %boop, [boop]
// CHECK-NEXT:  %1 = CreateFunctionInst %foo()
// CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [foo]
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [boop@main]
// CHECK-NEXT:  %1 = CondBranchInst %0, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
