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

// CHECK-LABEL: function foo#0#1()#2 : object
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:   %1 = CreateGeneratorInst %?anon_0_foo#1#2()#3, %0
// CHECK-NEXT:   %2 = ReturnInst %1 : object
// CHECK-NEXT: function_end

// CHECK-LABEL: function ?anon_0_foo#1#2()#3
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StartGeneratorInst
// CHECK-NEXT:   %1 = AllocStackInst $count
// CHECK-NEXT:   %2 = StoreStackInst undefined : undefined, %1 : undefined|number
// CHECK-NEXT:   %3 = CreateScopeInst %S{?anon_0_foo#1#2()#3}
// CHECK-NEXT:   %4 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:   %5 = ResumeGeneratorInst %4
// CHECK-NEXT:   %6 = LoadStackInst %4
// CHECK-NEXT:   %7 = CondBranchInst %6, %BB1, %BB2
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %8 = StoreStackInst 0 : number, %1 : undefined|number
// CHECK-NEXT:   %9 = TryStartInst %BB3, %BB4
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %10 = ReturnInst %5
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %11 = CatchInst
// CHECK-NEXT:   %12 = ThrowInst %11
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %13 = LoadStackInst %1 : undefined|number
// CHECK-NEXT:   %14 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:   %15 = SaveAndYieldInst %13 : undefined|number, %BB5
// CHECK-NEXT: %BB5:
// CHECK-NEXT:   %16 = ResumeGeneratorInst %14
// CHECK-NEXT:   %17 = LoadStackInst %14
// CHECK-NEXT:   %18 = CondBranchInst %17, %BB6, %BB7
// CHECK-NEXT: %BB7:
// CHECK-NEXT:   %19 = BranchInst %BB8
// CHECK-NEXT: %BB6:
// CHECK-NEXT:   %20 = BranchInst %BB9
// CHECK-NEXT: %BB9:
// CHECK-NEXT:   %21 = TryEndInst
// CHECK-NEXT:   %22 = ReturnInst %16
// CHECK-NEXT: %BB8:
// CHECK-NEXT:   %23 = TryEndInst
// CHECK-NEXT:   %24 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end
