// RUN: %hermes -commonjs %S/ -emit-binary -out %T/test.hbc && %hermes %T/test.hbc | %FileCheck --match-full-lines %s
// RUN: %hermes -commonjs -fstatic-require -fstatic-builtins %S/ -emit-binary -out %T/test.hbc && %hermes %T/test.hbc | %FileCheck --match-full-lines %s
// RUN: %hermes -O -commonjs -fstatic-require -fstatic-builtins %S/ -emit-binary -out %T/test.hbc && %hermes %T/test.hbc | %FileCheck --match-full-lines %s
// RUN: %hermes -O -commonjs -fstatic-require -fstatic-builtins %S/ -emit-binary -out %T/test.hbc && %hermes -b -dump-bytecode %T/test.hbc | %FileCheck --match-full-lines %s -check-prefix BC

print('main: init');
// CHECK-LABEL: main: init

loadSegment(require.context, 1);

var foo = require('./foo/cjs-subdir-foo.js');
// CHECK-NEXT: foo: init
// CHECK-NEXT: bar: init
// CHECK-NEXT: foo: bar.y = 15

print('main: foo.x =', foo.x);
// CHECK-NEXT: main: foo.x = 15

print(require('./cjs-subdir-2.js').alpha);
// CHECK-NEXT: 2: init
// CHECK-NEXT: 144

// BC-LABEL: String count: 10
// BC-LABEL: Global String Table:
// BC-NEXT:   i0[ASCII, {{.*}}] #{{.*}}: print
// BC-NEXT:   s1[ASCII, {{.*}}]: global
// BC-NEXT:   i2[ASCII, {{.*}}] #{{.*}}: alpha
// BC-NEXT:   s3[ASCII, {{.*}}]: cjs_module
// BC-NEXT:   s4[ASCII, {{.*}}]: 2: init
// BC-NEXT:   i5[ASCII, {{.*}}] #{{.*}}: context
// BC-NEXT:   i6[ASCII, {{.*}}] #{{.*}}: x
// BC-NEXT:   i7[ASCII, {{.*}}] #{{.*}}: loadSegment
// BC-NEXT:   s8[ASCII, {{.*}}]: main: foo.x =
// BC-NEXT:   s9[ASCII, {{.*}}]: main: init
