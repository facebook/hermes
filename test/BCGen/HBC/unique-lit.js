/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-bytecode %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -dump-bytecode %s | %FileCheck --match-full-lines  --check-prefix=CHKOPT %s

// Check that literals are emitted as needed without optimizations, and uniqued
// by CSE when optimizations are enabled.

var a, b;

function foo(x) {
  if (x)
    a = 10;
  else
    b = 10;
}

//CHECK-LABEL:Function<foo>(2 params, 4 registers, 1 symbols):
//CHECK-NEXT:Offset in debug table: source 0x{{.*}}, scope 0x{{.*}}
//CHECK-NEXT:    CreateEnvironment r0
//CHECK-NEXT:    LoadParam         r1, 1
//CHECK-NEXT:    StoreToEnvironment r0, 0, r1
//CHECK-NEXT:    LoadFromEnvironment r2, r0, 0
//CHECK-NEXT:    JmpTrue           L1, r2
//CHECK-NEXT:    LoadConstUInt8    r0, 10
//CHECK-NEXT:    GetGlobalObject   r1
//CHECK-NEXT:    PutById           r1, r0, 1, "b"
//CHECK-NEXT:    Jmp               L2
//CHECK-NEXT:L1:
//CHECK-NEXT:    LoadConstUInt8    r0, 10
//CHECK-NEXT:    GetGlobalObject   r1
//CHECK-NEXT:    PutById           r1, r0, 2, "a"
//CHECK-NEXT:L2:
//CHECK-NEXT:    LoadConstUndefined r0
//CHECK-NEXT:    Ret               r0

//CHKOPT-LABEL:Function<foo>(2 params, 3 registers, 0 symbols):
//CHKOPT-NEXT:Offset in debug table: source 0x{{.*}}, scope 0x{{.*}}
//CHKOPT-NEXT:    LoadConstUInt8    r1, 10
//CHKOPT-NEXT:    GetGlobalObject   r0
//CHKOPT-NEXT:    LoadParam         r2, 1
//CHKOPT-NEXT:    JmpTrue           L1, r2
//CHKOPT-NEXT:    PutById           r0, r1, 1, "b"
//CHKOPT-NEXT:    Jmp               L2
//CHKOPT-NEXT:L1:
//CHKOPT-NEXT:    PutById           r0, r1, 2, "a"
//CHKOPT-NEXT:L2:
//CHKOPT-NEXT:    LoadConstUndefined r0
//CHKOPT-NEXT:    Ret               r0
