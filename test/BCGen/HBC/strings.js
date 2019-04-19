// RUN: %hermes -target=HBC -dump-bytecode -O -fno-inline %s | %FileCheck --match-full-lines %s

//CHECK-LABEL:Global String Table:
//CHECK-NEXT:  s0[ASCII, 0..-1]:
//CHECK-NEXT:  i1[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: a
//CHECK-NEXT:  s2[ASCII, {{[0-9]+}}..{{[0-9]+}}]: abcdefg
//CHECK-NEXT:  s3[ASCII, {{[0-9]+}}..{{[0-9]+}}]: global
//CHECK-NEXT:  i4[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: b
//CHECK-NEXT:  s5[ASCII, {{[0-9]+}}..{{[0-9]+}}]: bar
//CHECK-NEXT:  i6[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: c
//CHECK-NEXT:  s7[ASCII, {{[0-9]+}}..{{[0-9]+}}]: cee
//CHECK-NEXT:  i8[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: e
//CHECK-NEXT:  i9[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: d
//CHECK-NEXT:  i10[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: foo
//CHECK-NEXT:  s11[ASCII, {{[0-9]+}}..{{[0-9]+}}]: 123
//CHECK-NEXT:  s12[ASCII, {{[0-9]+}}..{{[0-9]+}}]: \x00\x00ppp\x00\x00
//CHECK-NEXT:  s13[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \xE5\x00
//CHECK-NEXT:  s14[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x3F\x04\x40\x04\x38\x04\x32\x04\x35\x04\x42\x04

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
