/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

(function main() {
  for (var v1 = 0;; v1++) {
    switch (v1 === "") {
      case 0:
        for (var v10 = 0; v10 < 1; v10++) {
          switch (v1) {
            case 0:
              var v12 = 0;
              var v13 = 10;
              while (v12 < v13) {}
          }
        }
        break;
      default:
        break;
    }
  }
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:S{global#0()#1} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = PhiInst 0 : number, %BB0, %10 : number|bigint, %BB2
// CHECK-NEXT:  %3 = PhiInst undefined : undefined, %BB0, %7 : undefined, %BB2
// CHECK-NEXT:  %4 = PhiInst undefined : undefined, %BB0, %8 : undefined, %BB2
// CHECK-NEXT:  %5 = PhiInst undefined : undefined, %BB0, %9 : undefined, %BB2
// CHECK-NEXT:  %6 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = PhiInst %3 : undefined, %BB1
// CHECK-NEXT:  %8 = PhiInst %4 : undefined, %BB1
// CHECK-NEXT:  %9 = PhiInst %5 : undefined, %BB1
// CHECK-NEXT:  %10 = UnaryOperatorInst '++', %2 : number|bigint
// CHECK-NEXT:  %11 = BranchInst %BB1
// CHECK-NEXT:function_end
