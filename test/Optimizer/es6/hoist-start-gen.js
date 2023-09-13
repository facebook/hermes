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
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): object
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_foo(): any
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_foo(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       TryStartInst %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = CatchInst (:any)
// CHECK-NEXT:       ThrowInst %7: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst 0: number, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = ResumeGeneratorInst (:any) %9: boolean
// CHECK-NEXT:  %12 = LoadStackInst (:boolean) %9: boolean
// CHECK-NEXT:        CondBranchInst %12: boolean, %BB6, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
