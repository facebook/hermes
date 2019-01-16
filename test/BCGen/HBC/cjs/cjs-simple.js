// RUN: %hermes -commonjs -dump-bytecode %s | %FileCheck --match-full-lines %s

print('done');

// CHECK: Global String Table:
// CHECK-NEXT:   s0[ASCII, 0..14]: ./cjs-simple.js

// CHECK: CommonJS Modules:
// CHECK-NEXT:   File ID 0 -> function ID 1
