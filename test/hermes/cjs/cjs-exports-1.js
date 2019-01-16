// RUN: %hermes -commonjs %S/cjs-exports-1.js %S/cjs-exports-2.js | %FileCheck --match-full-lines %s
// RUN: %hermes -O -fstatic-require -fstatic-builtins -commonjs %S/cjs-exports-1.js %S/cjs-exports-2.js | %FileCheck --match-full-lines %s

print('1: init');
// CHECK-LABEL: 1: init

var mod2 = require('./cjs-exports-2.js');
// CHECK-NEXT: 2: init

print('1: mod2.x =', mod2.x);
// CHECK-NEXT: 1: mod2.x = 3
