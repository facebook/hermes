/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s --match-full-lines

function foo () {
  return;
  var a;
  for(;;)
    ++a;
}

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

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "bar" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %foo() : undefined
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %bar() : undefined
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "bar" : string
// CHECK-NEXT:  %6 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %7 = CallInst %6, undefined : undefined
// CHECK-NEXT:  %8 = LoadPropertyInst globalObject : object, "bar" : string
// CHECK-NEXT:  %9 = CallInst %8, undefined : undefined
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:function_end

// CHECK:function foo() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = CatchInst
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = TryEndInst
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
