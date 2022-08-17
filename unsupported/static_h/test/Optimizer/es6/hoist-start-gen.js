/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -Xflow-parser -dump-ir %s | %FileCheck --match-full-lines %s
// REQUIRES: flowparser

function *foo() {
  var count = 0;
  try {
    yield count;
  } finally {
  }
}

// CHECK-LABEL: function foo() : object
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT: %0 = CreateGeneratorInst %?anon_0_foo()
// CHECK-NEXT: %1 = ReturnInst %0 : object
// CHECK-NEXT: function_end

// CHECK-LABEL: function ?anon_0_foo()
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT: %0 = StartGeneratorInst
// CHECK-NEXT: %1 = AllocStackInst $count
// CHECK-NEXT: %2 = StoreStackInst undefined : undefined, %1 : undefined|number
// CHECK-NEXT: %3 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT: %4 = ResumeGeneratorInst %3
// CHECK-NEXT: %5 = LoadStackInst %3
// CHECK-NEXT: %6 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT: %BB2:
// CHECK-NEXT: %7 = StoreStackInst 0 : number, %1 : undefined|number
// CHECK-NEXT: %8 = TryStartInst %BB3, %BB4
// CHECK-NEXT: %BB1:
// CHECK-NEXT: %9 = ReturnInst %4
// CHECK-NEXT: %BB3:
// CHECK-NEXT: %10 = CatchInst
// CHECK-NEXT: %11 = ThrowInst %10
// CHECK-NEXT: %BB4:
// CHECK-NEXT: %12 = LoadStackInst %1 : undefined|number
// CHECK-NEXT: %13 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT: %14 = SaveAndYieldInst %12 : undefined|number, %BB5
// CHECK-NEXT: %BB5:
// CHECK-NEXT: %15 = ResumeGeneratorInst %13
// CHECK-NEXT: %16 = LoadStackInst %13
// CHECK-NEXT: %17 = CondBranchInst %16, %BB6, %BB7
// CHECK-NEXT: %BB7:
// CHECK-NEXT: %18 = BranchInst %BB8
// CHECK-NEXT: %BB6:
// CHECK-NEXT: %19 = BranchInst %BB9
// CHECK-NEXT: %BB9:
// CHECK-NEXT: %20 = TryEndInst
// CHECK-NEXT: %21 = ReturnInst %15
// CHECK-NEXT: %BB8:
// CHECK-NEXT: %22 = TryEndInst
// CHECK-NEXT: %23 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end
