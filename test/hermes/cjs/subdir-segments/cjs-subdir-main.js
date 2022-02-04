/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -commonjs %S/ -emit-binary -out %T/test.hbc && %hermes %T/test.hbc | %FileCheck --match-full-lines %s
// RUN: %hermes -commonjs -fstatic-require -fstatic-builtins %S/ -emit-binary -out %T/test.hbc && %hermes %T/test.hbc | %FileCheck --match-full-lines %s
// RUN: %hermes -O -commonjs -fstatic-require -fstatic-builtins %S/ -emit-binary -out %T/test.hbc && %hermes %T/test.hbc | %FileCheck --match-full-lines %s
// RUN: %hermes -O -commonjs -fstatic-require -fstatic-builtins %S/ -dump-bytecode | %FileCheck --match-full-lines %s -check-prefix BC
// RUN: %hermes -O -commonjs -fstatic-require -fstatic-builtins %S/ -emit-binary -out %T/test.hbc && %hermes -b %T/test.hbc.5 -dump-bytecode | %FileCheck --match-full-lines %s -check-prefix BC5
// RUN: %hermes -O -commonjs -fstatic-require -fstatic-builtins %S/ -emit-binary -out %T/test.hbc && %hermes -b %T/test.hbc.10 -dump-bytecode | %FileCheck --match-full-lines %s -check-prefix BC10
// RUN: %hermes -O -commonjs -fstatic-require -fstatic-builtins %S/ -emit-binary -out %T/test.hbc && cat %T/manifest.json | %FileCheck --match-full-lines %s -check-prefix MANIFEST

print('main: init');
// CHECK-LABEL: main: init

loadSegment(require.context, 10);

var foo = require('./foo/cjs-subdir-foo.js');
// CHECK-NEXT: foo: init
// CHECK-NEXT: bar: init
// CHECK-NEXT: foo: bar.y = 15
// CHECK-NEXT: shared: init

print('main: foo.x =', foo.x);
// CHECK-NEXT: main: foo.x = 15

loadSegment(require.context, 5);

var mod2 = require('./cjs-subdir-2.js');
print(mod2.alpha);
// CHECK-NEXT: 2: init
// CHECK-NOT: shared: init
// CHECK-NEXT: 144

print(mod2.shared === foo.shared);
// CHECK-NEXT: true
print(mod2.shared.value);
// CHECK-NEXT: 42

// BC-LABEL: String count: 11
// BC-LABEL: Global String Table:

// BC-NEXT:   s0[ASCII, {{.*}}]: cjs_module
// BC-NEXT:   s1[ASCII, {{.*}}]: global
// BC-NEXT:   s2[ASCII, {{.*}}]: main: foo.x =
// BC-NEXT:   s3[ASCII, {{.*}}]: main: init
// BC-NEXT:   i4[ASCII, {{.*}}] #{{.*}}: alpha
// BC-NEXT:   i5[ASCII, {{.*}}] #{{.*}}: context
// BC-NEXT:   i6[ASCII, {{.*}}] #{{.*}}: x
// BC-NEXT:   i7[ASCII, {{.*}}] #{{.*}}: loadSegment
// BC-NEXT:   i8[ASCII, {{.*}}] #{{.*}}: print
// BC-NEXT:   i9[ASCII, {{.*}}] #{{.*}}: shared
// BC-NEXT:   i10[ASCII, {{.*}}] #{{.*}}: value

// BC5-LABEL: CommonJS Modules (Static):
// BC5-NEXT: Module ID 4 -> function ID 1
// BC5-NEXT: Module ID 3 -> function ID 2

// BC10-LABEL: CommonJS Modules (Static):
// BC10-NEXT: Module ID 1 -> function ID 1
// BC10-NEXT: Module ID 2 -> function ID 2
// BC10-NEXT: Module ID 3 -> function ID 3

// BC5:    LoadConstString   {{.*}}, "shared: init"
// BC10:    LoadConstString   {{.*}}, "shared: init"

// MANIFEST-LABEL: [
// MANIFEST-NEXT:   {
// MANIFEST-NEXT:     "resource": "test.hbc",
// MANIFEST-NEXT:     "flavor": "hbc-seg-0",
// MANIFEST-NEXT:     "location": "test.hbc"
// MANIFEST-NEXT:   },
// MANIFEST-NEXT:   {
// MANIFEST-NEXT:     "resource": "test.hbc",
// MANIFEST-NEXT:     "flavor": "hbc-seg-10",
// MANIFEST-NEXT:     "location": "test.hbc.10"
// MANIFEST-NEXT:   },
// MANIFEST-NEXT:   {
// MANIFEST-NEXT:     "resource": "test.hbc",
// MANIFEST-NEXT:     "flavor": "hbc-seg-5",
// MANIFEST-NEXT:     "location": "test.hbc.5"
// MANIFEST-NEXT:   }
// MANIFEST-NEXT: ]
