// RUN: %hermes -commonjs -dump-bytecode %s | %FileCheck --match-full-lines %s

print('done');

// CHECK: Global String Table:
// CHECK:   s2[ASCII, {{.*}}]: cjs-simple.js

// CHECK: CommonJS Modules:
// CHECK-NEXT:   File ID 2 -> function ID 1
