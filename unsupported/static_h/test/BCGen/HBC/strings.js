/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -O -fno-inline %s | %FileCheck --match-full-lines %s

//CHECK-LABEL:Global String Table:
//CHECK-NEXT:  s0[ASCII, 0..-1]:
//CHECK-NEXT:  s1[ASCII, {{[0-9]+}}..{{[0-9]+}}]: \x00\x00ppp\x00\x00
//CHECK-NEXT:  s2[ASCII, {{[0-9]+}}..{{[0-9]+}}]: 123
//CHECK-NEXT:  s3[ASCII, {{[0-9]+}}..{{[0-9]+}}]: abcdefg
//CHECK-NEXT:  s4[ASCII, {{[0-9]+}}..{{[0-9]+}}]: global
//CHECK-NEXT:  s5[ASCII, {{[0-9]+}}..{{[0-9]+}}]: bar
//CHECK-NEXT:  s6[ASCII, {{[0-9]+}}..{{[0-9]+}}]: cee
//CHECK-NEXT:  s7[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \xE5\x00
//CHECK-NEXT:  s8[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x3F\x04\x40\x04\x38\x04\x32\x04\x35\x04\x42\x04
//CHECK-NEXT:  i9[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: a
//CHECK-NEXT:  i10[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: b
//CHECK-NEXT:  i11[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: c
//CHECK-NEXT:  i12[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: e
//CHECK-NEXT:  i13[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: d
//CHECK-NEXT:  i14[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: foo

var a = "123";
var b = "abcdefg";
var c = "\0\0ppp\0\0";
var d = "å";
var e = "привет";

function foo() {
  function bar() {
    function cee() {
    }
    var unnamed = [function() {}]
    cee();
    unnamed[0]();
  }
  bar();
}
foo();
