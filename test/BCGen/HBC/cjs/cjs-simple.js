// RUN: %hermes -commonjs -dump-bytecode %s | %FileCheck --match-full-lines %s

print('done');

// CHECK: Global String Table:
// CHECK:   s0[ASCII, {{.*}}]: cjs-simple.js

// CHECK: CommonJS Modules:
// CHECK-NEXT:   File ID 0 -> function ID 1
