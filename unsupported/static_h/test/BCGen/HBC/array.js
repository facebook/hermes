/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=true -O %s | %FileCheck --match-full-lines %s

var x = [true, false, 0, 1, undefined, null];
var y = ["foo", "foo", "bar",,,];
var z = [{}];

//CHECK-LABEL:Global String Table:
//CHECK-NEXT:  s0[ASCII, {{[0-9]+\.\.[0-9]+}}]: bar
//CHECK-NEXT:  s1[ASCII, {{[0-9]+\.\.[0-9]+}}]: foo
//CHECK-NEXT:  s2[ASCII, {{[0-9]+\.\.[0-9]+}}]: global
//CHECK-NEXT:  i3[ASCII, {{[0-9]+\.\.[0-9]+}}] #{{[0-9A-Z]+}}: length
//CHECK-NEXT:  i4[ASCII, {{[0-9]+\.\.[0-9]+}}] #{{[0-9A-Z]+}}: x
//CHECK-NEXT:  i5[ASCII, {{[0-9]+\.\.[0-9]+}}] #{{[0-9A-Z]+}}: y
//CHECK-NEXT:  i6[ASCII, {{[0-9]+\.\.[0-9]+}}] #{{[0-9A-Z]+}}: z


//CHECK-LABEL:Array Buffer:
//CHECK-NEXT:true
//CHECK-NEXT:false
//CHECK-NEXT:[int 0]
//CHECK-NEXT:[int 1]
//CHECK-NEXT:[String 1]
//CHECK-NEXT:[String 1]
//CHECK-NEXT:[String 0]

//CHECK-LABEL:Function<global>{{.*}}:
//CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
//CHECK-NEXT:    DeclareGlobalVar  "x"
//CHECK-NEXT:    DeclareGlobalVar  "y"
//CHECK-NEXT:    DeclareGlobalVar  "z"
//CHECK-NEXT:    NewArrayWithBuffer r1, 6, 4, 0
//CHECK-NEXT:    LoadConstUndefined r0
//CHECK-NEXT:    PutOwnByIndex     r1, r0, 4
//CHECK-NEXT:    LoadConstNull     r2
//CHECK-NEXT:    PutOwnByIndex     r1, r2, 5
//CHECK-NEXT:    GetGlobalObject   r2
//CHECK-NEXT:    PutById           r2, r1, 1, "x"
//CHECK-NEXT:    NewArrayWithBuffer r1, 5, 3, 11
//CHECK-NEXT:    LoadConstUInt8    r3, 5
//CHECK-NEXT:    PutById           r1, r3, 2, "length"
//CHECK-NEXT:    PutById           r2, r1, 3, "y"
//CHECK-NEXT:    NewArray          r1, 1
//CHECK-NEXT:    NewObject         r3
//CHECK-NEXT:    PutOwnByIndex     r1, r3, 0
//CHECK-NEXT:    PutById           r2, r1, 4, "z"
//CHECK-NEXT:    Ret               r0
