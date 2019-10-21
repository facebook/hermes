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
//CHKRA-LABEL:%BB86:
//CHKRA-NEXT:  {{.*}}  %256 = LoadPropertyInst %11, "return" : string
//CHKRA-NEXT:  {{.*}}  %257 = CompareBranchInst '===', %256, %1 : undefined, %BB1, %BB87
//CHKRA-LABEL:%BB7:
//CHKRA-NEXT:  {{.*}}  %262 = LoadPropertyInst %11, "return" : string
//CHKRA-NEXT:  {{.*}}  %263 = CompareBranchInst '===', %262, %1 : undefined, %BB6, %BB88
