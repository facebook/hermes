/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -emit-binary -target=HBC -out=%t %s && %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O -base-bytecode %t %s | %FileCheck --match-full-lines %s

// This file acts as a base source code to test compiling other files with
// delta optimizing mode; also compile itself with delta optimizing mode, with
// the normally compiled bytecode as base bytecode, to test the case when no
// new strings are added in the new bytecode.

var a = "a";
var b = "b";
var pi = 'π';
var sigma = '𝚺';
var gamma = '𝚪';
var invalid_surrogate_pair = '\ud8d3\ud000';
var invalid_single_surrogate = '\ud800';
var obj = {'key1': 'val1', 'key2': '你好'};

//CHECK-LABEL:Global String Table:
//CHECK-NEXT:  s0[ASCII, {{[0-9]+}}..{{[0-9]+}}]: global
//CHECK-NEXT:  s1[ASCII, {{[0-9]+}}..{{[0-9]+}}]: val1
//CHECK-NEXT:  s2[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \xC0\x03
//CHECK-NEXT:  s3[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x60\x4F\x7D\x59
//CHECK-NEXT:  s4[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x00\xD8
//CHECK-NEXT:  s5[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x35\xD8\xAA\xDE
//CHECK-NEXT:  s6[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x35\xD8\xBA\xDE
//CHECK-NEXT:  s7[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \xD3\xD8\x00\xD0
//CHECK-NEXT:  i8[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: a
//CHECK-NEXT:  i9[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: obj
//CHECK-NEXT:  i10[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: b
//CHECK-NEXT:  i11[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: gamma
//CHECK-NEXT:  i12[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: pi
//CHECK-NEXT:  i13[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: invalid_single_surrogate
//CHECK-NEXT:  i14[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: invalid_surrogate_pair
//CHECK-NEXT:  i15[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: key1
//CHECK-NEXT:  i16[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: key2
//CHECK-NEXT:  i17[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: sigma
