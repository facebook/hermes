// RUN: (! %hermes -commonjs %S/cjs-error-multiple-1.js %S/cjs-error-multiple-2.js 2>&1 ) | %FileCheck %s

require('./cjs-error-multiple-2.js');

// CHECK: {{.*}}cjs-error-multiple-2.js:2:5: error: 'identifier' expected in variable declaration
// CHECK-NEXT: var = 3;
// CHECK-NEXT: ~~~~^
