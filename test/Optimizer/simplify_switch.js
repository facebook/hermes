/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheck %s --match-full-lines

//CHECK-LABEL:function global() : undefined
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:   %1 = SwitchInst %0, %BB1, 2 : number, %BB2, 44 : number, %BB2
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %2 = TryLoadGlobalPropertyInst globalObject : object, "cond" : string
//CHECK-NEXT:   %3 = CondBranchInst %2, %BB3, %BB1
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   %4 = TryLoadGlobalPropertyInst globalObject : object, "cond" : string
//CHECK-NEXT:   %5 = CondBranchInst %4, %BB3, %BB1
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %6 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
switch (x) { case 2:
     case 44:while (cond);
}
switch (8) { case 2: 6
   case 44:while (cond);
}

