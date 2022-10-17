/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s
// RUN: %hermes -O -Xdump-between-passes %s 2>&1 | %FileCheck --match-full-lines %s --check-prefix CHKIR
// RUN: %hermes -O -dump-ra %s | %FileCheck --match-full-lines %s --check-prefix CHKRA

// This test exercises an issue found in LowerArgumentsArray in which PHI nodes
// were not being properly updated.
function decrementArguments() {
    for (var i = 0; i < 2; i++) {
        var var1 = () => var3;
        var var3 = arguments;
    }
    return var3 - 1;
}

print(decrementArguments());


// CHKIR-LABEL: *** AFTER LowerConstruction
// CHKIR-LABEL: function decrementArguments#0#1()#2 : number
// CHKIR-LABEL: %BB0:
// CHKIR-LABEL: %BB1:
// CHKIR-NEXT:   %4 = PhiInst undefined : undefined, %BB0, %2 : object, %BB1
// CHKIR-NEXT:   %5 = PhiInst 0 : number, %BB0, %6 : number|bigint, %BB1
// CHKIR-LABEL: %BB2:
// CHKIR-LABEL: *** AFTER LowerArgumentsArray
// CHKIR-LABEL: function decrementArguments#0#1()#2 : number
// CHKIR-LABEL: %BB0:
// CHKIR-LABEL: %BB1:
// CHKIR-NEXT:   %5 = PhiInst undefined : undefined, %BB0, %15, %BB2
//                 N.B. the broken hermesc would fail to update %BB2 in
//                 the next instruction, thus having an invalid PHI node with an
//                 operand that wasn't a predecessor node.
// CHKIR-NEXT:   %6 = PhiInst 0 : number, %BB0, %7 : number|bigint, %BB2
// CHKIR-LABEL: %BB3:
// CHKIR-LABEL: %BB2:

// CHKRA-LABEL: function decrementArguments#0#1()#2 : number
// CHKRA-LABEL: %BB0:
// CHKRA-LABEL: %BB1:
// CHKRA-NEXT:   $Reg2 @7 [2...19)    %7 = PhiInst %5 : number, %BB0, %17 : number|bigint, %BB2
// CHKRA-NEXT:   $Reg2 @8 [9...18)   %8 = UnaryOperatorInst '++', %7 : number|bigint
// CHKRA-LABEL: %BB3:
// CHKRA-LABEL: %BB2:
// CHKRA-NEXT:   $Reg3 @15 [empty]    %15 = HBCReifyArgumentsInst %3
// CHKRA-NEXT:   $Reg3 @16 [empty]    %16 = LoadStackInst %3
// CHKRA-NEXT:   $Reg2 @17 [18...19)  %17 = MovInst %8 : number|bigint
