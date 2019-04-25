// RUN: %hermes -commonjs -dump-bytecode %s | %FileCheck --match-full-lines %s

print('done');

// CHECK: Global String Table:
// CHECK:   s3[ASCII, {{.*}}]: cjs-simple.js

// CHECK: CommonJS Modules:
// CHECK-NEXT:   File ID 3 -> function ID 1
