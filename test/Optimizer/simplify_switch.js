/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

switch (x) { case 2:
     case 44:while (cond);
}
switch (8) { case 2: 6
   case 44:while (cond);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %2 = SwitchInst %1, %BB1, 2 : number, %BB2, 44 : number, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "cond" : string
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB3, %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "cond" : string
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB3, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
