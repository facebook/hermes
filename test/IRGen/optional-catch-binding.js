/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo(f, g) {
  try {
    return f();
  } catch {
    return g();
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(f, g)#2
// CHECK-NEXT:frame = [f#2, g#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %f, [f#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %g, [g#2], %0
// CHECK-NEXT:  %3 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst
// CHECK-NEXT:  %5 = LoadFrameInst [g#2], %0
// CHECK-NEXT:  %6 = CallInst %5, undefined : undefined
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = LoadFrameInst [f#2], %0
// CHECK-NEXT:  %10 = CallInst %9, undefined : undefined
// CHECK-NEXT:  %11 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = TryEndInst
// CHECK-NEXT:  %13 = ReturnInst %10
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %15 = TryEndInst
// CHECK-NEXT:  %16 = BranchInst %BB3
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:function_end
