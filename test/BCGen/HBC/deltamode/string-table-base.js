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
//CHECK-NEXT:  s0[ASCII, {{[0-9]+}}..{{[0-9]+}}]: gamma
//CHECK-NEXT:  s1[ASCII, {{[0-9]+}}..{{[0-9]+}}]: a
//CHECK-NEXT:  s2[ASCII, {{[0-9]+}}..{{[0-9]+}}]: obj
//CHECK-NEXT:  s3[ASCII, {{[0-9]+}}..{{[0-9]+}}]: b
//CHECK-NEXT:  s4[ASCII, {{[0-9]+}}..{{[0-9]+}}]: pi
//CHECK-NEXT:  s5[ASCII, {{[0-9]+}}..{{[0-9]+}}]: invalid_single_surrogate
//CHECK-NEXT:  s6[ASCII, {{[0-9]+}}..{{[0-9]+}}]: invalid_surrogate_pair
//CHECK-NEXT:  s7[ASCII, {{[0-9]+}}..{{[0-9]+}}]: sigma
//CHECK-NEXT:  s8[ASCII, {{[0-9]+}}..{{[0-9]+}}]: global
//CHECK-NEXT:  s9[ASCII, {{[0-9]+}}..{{[0-9]+}}]: key1
//CHECK-NEXT:  s10[ASCII, {{[0-9]+}}..{{[0-9]+}}]: key2
//CHECK-NEXT:  s11[ASCII, {{[0-9]+}}..{{[0-9]+}}]: val1
//CHECK-NEXT:  s12[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \xC0\x03
//CHECK-NEXT:  s13[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x60\x4F\x7D\x59
//CHECK-NEXT:  s14[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x00\xD8
//CHECK-NEXT:  s15[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x35\xD8\xAA\xDE
//CHECK-NEXT:  s16[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x35\xD8\xBA\xDE
//CHECK-NEXT:  s17[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \xD3\xD8\x00\xD0
