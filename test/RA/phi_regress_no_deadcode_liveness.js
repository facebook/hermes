/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O --target=HBC -dump-ra %s | %FileCheck %s --check-prefix=CHKRA --match-full-lines
// Ensure that phi node does not update deadcode liveness interval
function b(d=([[[[{z:[{}]}]]]]=arguments)) {}

//CHKRA-LABEL:function b#0#1(d)#2 : undefined
//CHKRA-NEXT:frame = []
//CHKRA-LABEL:%BB1:
//CHKRA-NEXT:  {{.*}}  %17 = ReturnInst %1 : undefined
//CHKRA-LABEL:%BB68:
//CHKRA-NEXT:  $Reg16 @148 [empty]   %208 = IteratorCloseInst %5, false : boolean
//CHKRA-NEXT:  $Reg16 @149 [empty]   %209 = BranchInst %BB1
//CHKRA-LABEL:%BB7:
//CHKRA-NEXT:  $Reg2 @209 [empty]    %210 = IteratorCloseInst %5, true : boolean
//CHKRA-NEXT:  $Reg2 @210 [empty]    %211 = BranchInst %BB6
