/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheck --match-full-lines %s
"use strict";

//CHECK-LABEL:Function<global>{{.*}}:
//CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
//CHECK-NEXT:    DeclareGlobalVar  "foo"
//CHECK-NEXT:    CreateEnvironment r0
//CHECK-NEXT:    CreateClosure     r1, r0, Function<foo>
//CHECK-NEXT:    GetGlobalObject   r0
//CHECK-NEXT:    PutById           r0, r1, 1, "foo"
//CHECK-NEXT:    GetByIdShort      r3, r0, 1, "foo"
//CHECK-NEXT:    LoadConstUndefined r2
//CHECK-NEXT:    LoadConstZero     r1
//CHECK-NEXT:    LoadConstUInt8    r0, 1
//CHECK-NEXT:    Call4             r0, r3, r2, r1, r1, r0
//CHECK-NEXT:    Ret               r0

function foo(x, y, z) { }
foo(0,0,1);
