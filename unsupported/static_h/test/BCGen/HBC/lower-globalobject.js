/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ra -O %s | %FileCheck --match-full-lines --check-prefix=CHKRA %s
// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheck --match-full-lines --check-prefix=CHKBC %s

var x;

function foo() {
  return x;
}

//CHKRA-LABEL:function foo()
//CHKRA-NEXT:frame = []
//CHKRA-NEXT:%BB0:
//CHKRA-NEXT:  $Reg0 @0 [1...2) 	%0 = HBCGetGlobalObjectInst
//CHKRA-NEXT:  $Reg0 @1 [2...3) 	%1 = LoadPropertyInst %0 : object, "x" : string
//CHKRA-NEXT:  $Reg0 @2 [empty]	%2 = ReturnInst %1
//CHKRA-NEXT:function_end

//CHKBC-LABEL:Function<foo>(1 params, 1 registers, 0 symbols):
//CHKBC-NEXT:Offset in debug table:{{.*}}
//CHKBC-NEXT:    GetGlobalObject   r0
//CHKBC-NEXT:    GetByIdShort      r0, r0, 1, "x"
//CHKBC-NEXT:    Ret               r0
