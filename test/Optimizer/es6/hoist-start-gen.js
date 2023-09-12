/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function *foo() {
  var count = 0;
  try {
    yield count;
  } finally {
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): object
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_foo(): any
// CHECK-NEXT:  %1 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_foo(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:  %4 = CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = TryStartInst %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = CatchInst (:any)
// CHECK-NEXT:  %8 = ThrowInst %7: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:  %10 = SaveAndYieldInst 0: number, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = ResumeGeneratorInst (:any) %9: boolean
// CHECK-NEXT:  %12 = LoadStackInst (:boolean) %9: boolean
// CHECK-NEXT:  %13 = CondBranchInst %12: boolean, %BB6, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %14 = BranchInst %BB8
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %15 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %16 = TryEndInst
// CHECK-NEXT:  %17 = ReturnInst %11: any
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %18 = TryEndInst
// CHECK-NEXT:  %19 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
