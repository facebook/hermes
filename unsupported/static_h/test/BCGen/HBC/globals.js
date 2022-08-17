/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -O -strict %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -dump-bytecode -O -non-strict %s | %FileCheck --match-full-lines --check-prefix=CHKNONSTRICT %s

var x = 5;
foo(x);
y = x;

//CHECK-LABEL:Function<global>{{.*}}:
//CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
//CHECK-NEXT:    DeclareGlobalVar  "x"
//CHECK-NEXT:    LoadConstUInt8    r0, 5
//CHECK-NEXT:    GetGlobalObject   r1
//CHECK-NEXT:    PutById           r1, r0, 1, "x"
//CHECK-NEXT:    TryGetById        r3, r1, 1, "foo"
//CHECK-NEXT:    GetByIdShort      r2, r1, 2, "x"
//CHECK-NEXT:    LoadConstUndefined r0
//CHECK-NEXT:    Call2             r0, r3, r0, r2
//CHECK-NEXT:    GetByIdShort      r0, r1, 2, "x"
//CHECK-NEXT:    TryPutById        r1, r0, 2, "y"
//CHECK-NEXT:    Ret               r0

//CHKNONSTRICT-LABEL:Function<global>{{.*}}:
//CHKNONSTRICT-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
//CHKNONSTRICT-NEXT:    DeclareGlobalVar  "x"
//CHKNONSTRICT-NEXT:    LoadConstUInt8    r0, 5
//CHKNONSTRICT-NEXT:    GetGlobalObject   r1
//CHKNONSTRICT-NEXT:    PutById           r1, r0, 1, "x"
//CHKNONSTRICT-NEXT:    TryGetById        r3, r1, 1, "foo"
//CHKNONSTRICT-NEXT:    GetByIdShort      r2, r1, 2, "x"
//CHKNONSTRICT-NEXT:    LoadConstUndefined r0
//CHKNONSTRICT-NEXT:    Call2             r0, r3, r0, r2
//CHKNONSTRICT-NEXT:    GetByIdShort      r0, r1, 2, "x"
//CHKNONSTRICT-NEXT:    PutById           r1, r0, 2, "y"
//CHKNONSTRICT-NEXT:    Ret               r0
