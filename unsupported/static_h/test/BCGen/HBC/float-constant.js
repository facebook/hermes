/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=true -O %s | %FileCheck --match-full-lines %s

var w = 3.14;
var x = -0.00056;
var y = 12345670.89;
var z = 0.0;

//CHECK-LABEL:Function<global>{{.*}}:
//CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
//CHECK-NEXT:    DeclareGlobalVar  "w"
//CHECK-NEXT:    DeclareGlobalVar  "x"
//CHECK-NEXT:    DeclareGlobalVar  "y"
//CHECK-NEXT:    DeclareGlobalVar  "z"
//CHECK-NEXT:    LoadConstDouble   r0, 3.14
//CHECK-NEXT:    GetGlobalObject   r1
//CHECK-NEXT:    PutById           r1, r0, 1, "w"
//CHECK-NEXT:    LoadConstDouble   r0, -0.00056
//CHECK-NEXT:    PutById           r1, r0, 2, "x"
//CHECK-NEXT:    LoadConstDouble   r0, 12345670.89
//CHECK-NEXT:    PutById           r1, r0, 3, "y"
//CHECK-NEXT:    LoadConstZero     r0
//CHECK-NEXT:    PutById           r1, r0, 4, "z"
//CHECK-NEXT:    LoadConstUndefined r0
//CHECK-NEXT:    Ret               r0
