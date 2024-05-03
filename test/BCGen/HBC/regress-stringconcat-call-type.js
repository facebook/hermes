/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-lir %s | %FileCheckOrRegen --match-full-lines %s

(function a(bar) {
  var content = "";
  for (;;) {
    content += " ";
    content += content + " ";
  }
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any [noReturn]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCLoadConstInst (:string) "": string
// CHECK-NEXT:  %1 = HBCLoadConstInst (:string) " ": string
// CHECK-NEXT:  %2 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  %3 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst (:string) %0: string, %BB0, %9: string, %BB1
// CHECK-NEXT:  %6 = HBCStringConcatInst (:string) %5: string, %1: string
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) %2: object, "HermesInternal": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "concat": string
// CHECK-NEXT:  %9 = HBCCallNInst (:string) %8: any, empty: any, empty: any, %3: undefined, %6: string, %6: string, %1: string
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end
