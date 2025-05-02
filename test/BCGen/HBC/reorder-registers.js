/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -target=HBC -Xdump-functions=foo -freorder-registers -dump-lra -O %s | %FileCheckOrRegen --match-full-lines %s --check-prefix=REORDER
// RUN: %hermesc -target=HBC -Xdump-functions=foo -fno-reorder-registers -dump-lra -O %s | %FileCheckOrRegen --match-full-lines %s --check-prefix=NO-REORDER

function foo() {
  var prod = 1;
  for (var i = 1; i < 10; ++i) {
    for (var j = 1; j < 10; ++j) {
      // Observe that the registers for FMultiplyInst are lower indexed
      // when reordering is enabled.
      prod *= j;
    }
  }
}

// Auto-generated content below. Please do not modify manually.

// REORDER:function foo(): undefined
// REORDER-NEXT:%BB0:
// REORDER-NEXT:  {n3}      %0 = LIRLoadConstInst (:number) 1: number
// REORDER-NEXT:  {n7}      %1 = LIRLoadConstInst (:number) 10: number
// REORDER-NEXT:  {n4}      %2 = MovInst (:number) {n3} %0: number
// REORDER-NEXT:  {n5}      %3 = MovInst (:number) {n4} %2: number
// REORDER-NEXT:                 BranchInst %BB1
// REORDER-NEXT:%BB1:
// REORDER-NEXT:  {n4}      %5 = PhiInst (:number) {n4} %2: number, %BB0, {n4} %14: number, %BB3
// REORDER-NEXT:  {n5}      %6 = PhiInst (:number) {n5} %3: number, %BB0, {n5} %15: number, %BB3
// REORDER-NEXT:  {n6}      %7 = MovInst (:number) {n4} %5: number
// REORDER-NEXT:  {n1}      %8 = MovInst (:number) {n5} %6: number
// REORDER-NEXT:  {n0}      %9 = MovInst (:number) {n3} %0: number
// REORDER-NEXT:  {n1}     %10 = MovInst (:number) {n1} %8: number
// REORDER-NEXT:                 BranchInst %BB4
// REORDER-NEXT:%BB2:
// REORDER-NEXT:  {np0}    %12 = LIRLoadConstInst (:undefined) undefined: undefined
// REORDER-NEXT:                 ReturnInst {np0} %12: undefined
// REORDER-NEXT:%BB3:
// REORDER-NEXT:  {n4}     %14 = FAddInst (:number) {n6} %7: number, {n3} %0: number
// REORDER-NEXT:  {n5}     %15 = MovInst (:number) {n2} %19: number
// REORDER-NEXT:                 HBCFCmpBrLessThanInst {n4} %14: number, {n7} %1: number, %BB1, %BB2
// REORDER-NEXT:%BB4:
// REORDER-NEXT:  {n0}     %17 = PhiInst (:number) {n0} %9: number, %BB1, {n0} %20: number, %BB4
// REORDER-NEXT:  {n1}     %18 = PhiInst (:number) {n1} %10: number, %BB1, {n1} %21: number, %BB4
// REORDER-NEXT:  {n2}     %19 = FMultiplyInst (:number) {n1} %18: number, {n0} %17: number
// REORDER-NEXT:  {n0}     %20 = FAddInst (:number) {n0} %17: number, {n3} %0: number
// REORDER-NEXT:  {n1}     %21 = MovInst (:number) {n2} %19: number
// REORDER-NEXT:                 HBCFCmpBrLessThanInst {n0} %20: number, {n7} %1: number, %BB4, %BB3
// REORDER-NEXT:function_end

// NO-REORDER:function foo(): undefined
// NO-REORDER-NEXT:%BB0:
// NO-REORDER-NEXT:  {n3}      %0 = LIRLoadConstInst (:number) 1: number
// NO-REORDER-NEXT:  {n2}      %1 = LIRLoadConstInst (:number) 10: number
// NO-REORDER-NEXT:  {n1}      %2 = MovInst (:number) {n3} %0: number
// NO-REORDER-NEXT:  {n0}      %3 = MovInst (:number) {n1} %2: number
// NO-REORDER-NEXT:                 BranchInst %BB1
// NO-REORDER-NEXT:%BB1:
// NO-REORDER-NEXT:  {n1}      %5 = PhiInst (:number) {n1} %2: number, %BB0, {n1} %14: number, %BB3
// NO-REORDER-NEXT:  {n0}      %6 = PhiInst (:number) {n0} %3: number, %BB0, {n0} %15: number, %BB3
// NO-REORDER-NEXT:  {n4}      %7 = MovInst (:number) {n1} %5: number
// NO-REORDER-NEXT:  {n7}      %8 = MovInst (:number) {n0} %6: number
// NO-REORDER-NEXT:  {n6}      %9 = MovInst (:number) {n3} %0: number
// NO-REORDER-NEXT:  {n7}     %10 = MovInst (:number) {n7} %8: number
// NO-REORDER-NEXT:                 BranchInst %BB4
// NO-REORDER-NEXT:%BB2:
// NO-REORDER-NEXT:  {np0}    %12 = LIRLoadConstInst (:undefined) undefined: undefined
// NO-REORDER-NEXT:                 ReturnInst {np0} %12: undefined
// NO-REORDER-NEXT:%BB3:
// NO-REORDER-NEXT:  {n1}     %14 = FAddInst (:number) {n4} %7: number, {n3} %0: number
// NO-REORDER-NEXT:  {n0}     %15 = MovInst (:number) {n5} %19: number
// NO-REORDER-NEXT:                 HBCFCmpBrLessThanInst {n1} %14: number, {n2} %1: number, %BB1, %BB2
// NO-REORDER-NEXT:%BB4:
// NO-REORDER-NEXT:  {n6}     %17 = PhiInst (:number) {n6} %9: number, %BB1, {n6} %20: number, %BB4
// NO-REORDER-NEXT:  {n7}     %18 = PhiInst (:number) {n7} %10: number, %BB1, {n7} %21: number, %BB4
// NO-REORDER-NEXT:  {n5}     %19 = FMultiplyInst (:number) {n7} %18: number, {n6} %17: number
// NO-REORDER-NEXT:  {n6}     %20 = FAddInst (:number) {n6} %17: number, {n3} %0: number
// NO-REORDER-NEXT:  {n7}     %21 = MovInst (:number) {n5} %19: number
// NO-REORDER-NEXT:                 HBCFCmpBrLessThanInst {n6} %20: number, {n2} %1: number, %BB4, %BB3
// NO-REORDER-NEXT:function_end
