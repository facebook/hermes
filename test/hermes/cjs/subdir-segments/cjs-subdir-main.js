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
// BC-NEXT:   i0[ASCII, 0..4] #{{.*}}: print
// BC-NEXT:   i1[ASCII, 5..9] #{{.*}}: alpha
// BC-NEXT:   s2[ASCII, 10..19]: cjs_module
// BC-NEXT:   s3[ASCII, 20..26]: 2: init
// BC-NEXT:   i4[ASCII, 27..33] #{{.*}}: context
// BC-NEXT:   i5[ASCII, 32..32] #{{.*}}: x
// BC-NEXT:   i6[ASCII, 34..44] #{{.*}}: loadSegment
// BC-NEXT:   s7[ASCII, 45..57]: main: foo.x =
// BC-NEXT:   s8[ASCII, 58..67]: main: init
// BC-NEXT:   s9[ASCII, 68..73]: global
