/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O --target=HBC -dump-ra %s | %FileCheck %s --check-prefix=CHKRA --match-full-lines
// Ensure that phi node does not update deadcode liveness interval
function b(d=([[[[{z:[{}]}]]]]=arguments)) {}

//CHKRA-LABEL:function b(d) : undefined
//CHKRA-NEXT:frame = []
//CHKRA-LABEL:%BB1:
//CHKRA-NEXT:  {{.*}}  %20 = ReturnInst %1 : undefined
//CHKRA-LABEL:%BB88:
//CHKRA-NEXT:  {{.*}}  %265 = LoadPropertyInst %11, "return" : string
//CHKRA-NEXT:  {{.*}}  %266 = CompareBranchInst '===', %265, %1 : undefined, %BB1, %BB89
//CHKRA-LABEL:%BB7:
//CHKRA-NEXT:  {{.*}}  %271 = LoadPropertyInst %11, "return" : string
//CHKRA-NEXT:  {{.*}}  %272 = CompareBranchInst '===', %271, %1 : undefined, %BB6, %BB90
