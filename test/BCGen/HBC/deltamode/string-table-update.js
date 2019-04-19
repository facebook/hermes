// RUN: %hermes -O -emit-binary -target=HBC -out=%t %S/string-table-base.js && %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O -base-bytecode %t %s | %FileCheck --match-full-lines %s

// Compile with delta optimizing mode so the string table should contain all
// strings from string-table-base.js, and with all strings combined and
// correctly deduplicated.
var a = "a";
var c = "c";
var pi = 'π';
var sigma = '𝚺';
var invalid_surrogate_pair = '\ud8d3\ud000';
var invalid_single_surrogate = '\ud800';
var obj = {'key11': 'val1', 'key2': '再见'};
var unicode = '\u7231\u9a6c\u4ed5';
var unicode2 = '\u8138\u4e66';
var ascii = 'hello how are you';

//CHECK-LABEL:Global String Table:
//CHECK-NEXT:  i0[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: sigma
//CHECK-NEXT:  i1[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: a
//CHECK-NEXT:  i2[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: obj
//CHECK-NEXT:  i3[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: b
//CHECK-NEXT:  i4[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: pi
//CHECK-NEXT:  i5[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: invalid_surrogate_pair
//CHECK-NEXT:  i6[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: gamma
//CHECK-NEXT:  i7[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: invalid_single_surrogate
//CHECK-NEXT:  i8[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: key1
//CHECK-NEXT:  s9[ASCII, {{[0-9]+}}..{{[0-9]+}}]: val1
//CHECK-NEXT:  i10[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: key2
//CHECK-NEXT:  s11[ASCII, {{[0-9]+}}..{{[0-9]+}}]: global
//CHECK-NEXT:  s12[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \xC0\x03
//CHECK-NEXT:  s13[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x35\xD8\xBA\xDE
//CHECK-NEXT:  s14[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x35\xD8\xAA\xDE
//CHECK-NEXT:  s15[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \xD3\xD8\x00\xD0
//CHECK-NEXT:  s16[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x00\xD8
//CHECK-NEXT:  s17[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x60\x4F\x7D\x59
//CHECK-NEXT:  i18[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: c
//CHECK-NEXT:  s19[ASCII, {{[0-9]+}}..{{[0-9]+}}]: hello how are you
//CHECK-NEXT:  i20[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: unicode
//CHECK-NEXT:  i21[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: unicode2
//CHECK-NEXT:  i22[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: ascii
//CHECK-NEXT:  i23[ASCII, {{[0-9]+}}..{{[0-9]+}}] #{{[0-9A-Z]+}}: key11
//CHECK-NEXT:  s24[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x8D\x51\xC1\x89
//CHECK-NEXT:  s25[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x31\x72\x6C\x9A\xD5\x4E
//CHECK-NEXT:  s26[UTF-16, {{[0-9]+}}..{{[0-9]+}}]: \x38\x81\x66\x4E
//CHECK-NOT: string-table-update.js
//CHECK-LABEL: Function<global>(1 params, 2 registers, 0 symbols):
