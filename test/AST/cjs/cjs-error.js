// RUN: (! %hermes -commonjs %s 2>&1 ) | %FileCheck %s

var = 3;

// CHECK: {{.*}}cjs-error.js:3:5: error: 'identifier' expected in declaration
// CHECK-NEXT: var = 3;
// CHECK-NEXT: ~~~~^
