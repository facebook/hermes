/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ra -O %s | %FileCheck --match-full-lines %s

//CHECK-LABEL:function count()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}} %0 = AllocStackInst $arguments
//CHECK-NEXT:  {{.*}} %1 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  {{.*}} %2 = StoreStackInst %1 : undefined, %0
//CHECK-NEXT:  {{.*}} %3 = HBCGetArgumentsLengthInst %0
//CHECK-NEXT:  {{.*}} %4 = ReturnInst %3
//CHECK-NEXT:function_end
function count() {
  return arguments.length;
}

//CHECK-LABEL:function select(x)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}} %0 = AllocStackInst $arguments
//CHECK-NEXT:  {{.*}} %1 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  {{.*}} %2 = StoreStackInst %1 : undefined, %0
//CHECK-NEXT:  {{.*}} %3 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}} %4 = HBCLoadConstInst 1 : number
//CHECK-NEXT:  {{.*}} %5 = BinaryOperatorInst '+', %3, %4 : number
//CHECK-NEXT:  {{.*}} %6 = HBCGetArgumentsPropByValInst %5 : string|number, %0
//CHECK-NEXT:  {{.*}} %7 = ReturnInst %6
//CHECK-NEXT:function_end
function select(x) {
  return arguments[x+1];
}

//CHECK-LABEL:function build() : object
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}} %0 = AllocStackInst $arguments
//CHECK-NEXT:  {{.*}} %1 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  {{.*}} %2 = StoreStackInst %1 : undefined, %0
//CHECK-NEXT:  {{.*}} %3 = HBCReifyArgumentsInst %0
//CHECK-NEXT:  {{.*}} %4 = LoadStackInst %0
//CHECK-NEXT:  {{.*}} %5 = ReturnInst %4
//CHECK-NEXT:function_end
function build() {
  return arguments;
}


//CHECK-LABEL:function buffalobuffalo() : string|number
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  $Reg0 @0 [1...5)  %0 = AllocStackInst $arguments
//CHECK-NEXT:  $Reg1 @1 [2...3)  %1 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  $Reg1 @2 [empty]  %2 = StoreStackInst %1 : undefined, %0
//CHECK-NEXT:  $Reg1 @3 [empty]  %3 = HBCReifyArgumentsInst %0
//CHECK-NEXT:  $Reg0 @4 [5...6)  %4 = LoadStackInst %0
//CHECK-NEXT:  $Reg0 @5 [6...7)  %5 = BinaryOperatorInst '+', %4, %4
//CHECK-NEXT:  $Reg0 @6 [empty]  %6 = ReturnInst %5 : string|number
//CHECK-NEXT:function_end
function buffalobuffalo() {
  if(arguments) {
    return arguments + arguments;
  }
  return arguments;
}

//CHECK-LABEL:function check_phi_handling(x) : object
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}} %0 = AllocStackInst $arguments
//CHECK-NEXT:  {{.*}} %1 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  {{.*}} %2 = StoreStackInst %1 : undefined, %0
//CHECK-NEXT:  {{.*}} %3 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}} %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}} %5 = AllocArrayInst 1 : number, 1 : number
//CHECK-NEXT:  {{.*}} %6 = MovInst %5 : object
//CHECK-NEXT:  {{.*}} %7 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  {{.*}} %8 = PhiInst %6 : object, %BB1, %13, %BB2
//CHECK-NEXT:  {{.*}} %9 = MovInst %8 : object
//CHECK-NEXT:  {{.*}} %10 = ReturnInst %9 : object
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}} %11 = HBCReifyArgumentsInst %0
//CHECK-NEXT:  {{.*}} %12 = LoadStackInst %0
//CHECK-NEXT:  {{.*}} %13 = MovInst %12
//CHECK-NEXT:  {{.*}} %14 = BranchInst %BB3
//CHECK-NEXT:function_end
function check_phi_handling(x) {
  return x ? [1] : arguments;
}
