/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-postra %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -O -dump-postra %s | %FileCheckOrRegen --match-full-lines  --check-prefix=CHKOPT %s

// Check that literals are uniqued when optimizations is disabled, but aren't
// when it is enabled.

var a, b;

function foo(x) {
  if (x)
    a = 10;
  else
    b = 10;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [a, b, foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst
// CHECK-NEXT:  %1 = HBCGetGlobalObjectInst
// CHECK-NEXT:  %2 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  %3 = HBCCreateFunctionInst %foo(), %0
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3 : closure, %1 : object, "foo" : string
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %6 = MovInst %2 : undefined
// CHECK-NEXT:  %7 = LoadStackInst %5
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function foo(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst
// CHECK-NEXT:  %1 = HBCLoadConstInst 10 : number
// CHECK-NEXT:  %2 = HBCGetGlobalObjectInst
// CHECK-NEXT:  %3 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  %4 = LoadParamInst %x
// CHECK-NEXT:  %5 = HBCStoreToEnvironmentInst %0, %4, [x]
// CHECK-NEXT:  %6 = HBCLoadFromEnvironmentInst %0, [x]
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = StorePropertyLooseInst %1 : number, %2 : object, "a" : string
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = StorePropertyLooseInst %1 : number, %2 : object, "b" : string
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = ReturnInst %3 : undefined
// CHECK-NEXT:function_end

// CHKOPT:function global() : undefined
// CHKOPT-NEXT:frame = [], globals = [a, b, foo]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = HBCCreateEnvironmentInst
// CHKOPT-NEXT:  %1 = HBCCreateFunctionInst %foo() : undefined, %0
// CHKOPT-NEXT:  %2 = HBCGetGlobalObjectInst
// CHKOPT-NEXT:  %3 = StorePropertyLooseInst %1 : closure, %2 : object, "foo" : string
// CHKOPT-NEXT:  %4 = HBCLoadConstInst undefined : undefined
// CHKOPT-NEXT:  %5 = ReturnInst %4 : undefined
// CHKOPT-NEXT:function_end

// CHKOPT:function foo(x) : undefined
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = LoadParamInst %x
// CHKOPT-NEXT:  %1 = CondBranchInst %0, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %2 = HBCLoadConstInst 10 : number
// CHKOPT-NEXT:  %3 = HBCGetGlobalObjectInst
// CHKOPT-NEXT:  %4 = StorePropertyLooseInst %2 : number, %3 : object, "a" : string
// CHKOPT-NEXT:  %5 = BranchInst %BB3
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:  %6 = HBCLoadConstInst 10 : number
// CHKOPT-NEXT:  %7 = HBCGetGlobalObjectInst
// CHKOPT-NEXT:  %8 = StorePropertyLooseInst %6 : number, %7 : object, "b" : string
// CHKOPT-NEXT:  %9 = BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:  %10 = HBCLoadConstInst undefined : undefined
// CHKOPT-NEXT:  %11 = ReturnInst %10 : undefined
// CHKOPT-NEXT:function_end
