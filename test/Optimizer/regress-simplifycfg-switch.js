/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -custom-opt=simplestackpromotion -custom-opt=mem2reg -custom-opt=instsimplify -custom-opt=simplifycfg -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

// Test for a bug in SimplifyCFG, where -0 and +0 were incorrectly considered
// not equal in switch statement strict equality comparison.

switch (-0) {
  case 0:
    print("case")
    break;
  default:
    print("default")
    break;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:S{global#0()#1} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined, undefined : undefined, "case" : string
// CHECK-NEXT:  %4 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst %3, %BB0
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end
