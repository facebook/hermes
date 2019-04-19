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
//CHECK-NEXT:  i0[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: sigma
//CHECK-NEXT:  i1[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: a
//CHECK-NEXT:  i2[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: obj
//CHECK-NEXT:  i3[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: b
//CHECK-NEXT:  i4[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: pi
//CHECK-NEXT:  i5[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: invalid_surrogate_pair
//CHECK-NEXT:  i6[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: gamma
//CHECK-NEXT:  i7[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: invalid_single_surrogate
//CHECK-NEXT:  s8[ASCII, {{[0-9]+}}..{{[0-9]+}}]: global
//CHECK-NEXT:  i9[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: key1
//CHECK-NEXT:  s10[ASCII, {{[0-9]+}}..{{[0-9]+}}]: val1
//CHECK-NEXT:  i11[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: key2
//CHECK-NEXT:  s12[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \xC0\x03
//CHECK-NEXT:  s13[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x35\xD8\xBA\xDE
//CHECK-NEXT:  s14[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x35\xD8\xAA\xDE
//CHECK-NEXT:  s15[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \xD3\xD8\x00\xD0
//CHECK-NEXT:  s16[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x00\xD8
//CHECK-NEXT:  s17[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x60\x4F\x7D\x59
