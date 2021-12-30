/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes --target=HBC -O %s | %FileCheck %s --match-full-lines
// RUN: %hermes --target=HBC -O -dump-ra %s | %FileCheck %s --check-prefix=CHKRA --match-full-lines

// Using a Phi in a successor of a Phi predecessor block:
// B0:
//   ...
// B1:
//   %p = PhiInst %a, B0, %b, B2
//   ...
// B2:
//   CondBranchInst ..., B1, B3
// B3:
//   Use %p

var glob = null;

function bad(param1, param2) {
    for (;;) {
        if (param2)
            param2.foo = 0;
        if (!param2) {
            glob = param1;
            return null;
        }
        param1 = param2;
    }
    return null;
}

bad("foo", null);
print("phi");
//CHECK: phi
print(glob);
//CHECK-NEXT: foo

//CHKRA-LABEL:function bad(param1, param2) : null
//CHKRA-NEXT:frame = []
//CHKRA-NEXT:%BB0:
//CHKRA-NEXT:  $Reg3 @0 [1...4) 	%0 = HBCLoadParamInst 1 : number
//CHKRA-NEXT:  $Reg2 @1 [2...12) 	%1 = HBCLoadParamInst 2 : number
//CHKRA-NEXT:  $Reg0 @2 [3...12) 	%2 = HBCLoadConstInst 0 : number
//CHKRA-NEXT:  $Reg3 @3 [4...6) 	%3 = MovInst %0
//CHKRA-NEXT:  $Reg1 @4 [empty]	%4 = BranchInst %BB1
//CHKRA-NEXT:%BB1:
//CHKRA-NEXT:  $Reg3 @5 [1...7) [11...12) 	%5 = PhiInst %3, %BB0, %10, %BB2
//CHKRA-NEXT:  $Reg1 @6 [7...14) 	%6 = MovInst %5
//CHKRA-NEXT:  $Reg4 @7 [empty]	%7 = CondBranchInst %1, %BB3, %BB2
//CHKRA-NEXT:%BB3:
//CHKRA-NEXT:  $Reg4 @8 [empty]	%8 = StorePropertyInst %2 : number, %1, "foo" : string
//CHKRA-NEXT:  $Reg4 @9 [empty]	%9 = BranchInst %BB2
//CHKRA-NEXT:%BB2:
//CHKRA-NEXT:  $Reg3 @10 [11...12) 	%10 = MovInst %1
//CHKRA-NEXT:  $Reg0 @11 [empty]	%11 = CondBranchInst %10, %BB1, %BB4
//CHKRA-NEXT:%BB4:
//CHKRA-NEXT:  $Reg0 @12 [13...14) 	%12 = HBCGetGlobalObjectInst
//CHKRA-NEXT:  $Reg0 @13 [empty]	%13 = StorePropertyInst %6, %12 : object, "glob" : string
//CHKRA-NEXT:  $Reg0 @14 [15...16) 	%14 = HBCLoadConstInst null : null
//CHKRA-NEXT:  $Reg0 @15 [empty]	%15 = ReturnInst %14 : null
//CHKRA-NEXT:function_end
