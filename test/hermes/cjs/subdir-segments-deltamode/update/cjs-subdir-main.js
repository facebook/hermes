/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -commonjs -fstatic-require -fstatic-builtins %S/ -dump-bytecode | %FileCheck --match-full-lines %s -check-prefix BC

print('main: init');

loadSegment(require.context, 1);

var foo = require('./foo/cjs-subdir-foo.js');

print('main: foo.x =', foo.x);

print(require('./cjs-subdir-2.js').alpha);

// BC-LABEL: String count: 10
// BC-LABEL: Global String Table:
// BC-NEXT:   s0[ASCII, {{.*}}]: 2: init
// BC-NEXT:   s1[ASCII, {{.*}}]: cjs_module
// BC-NEXT:   s2[ASCII, {{.*}}]: global
// BC-NEXT:   s3[ASCII, {{.*}}]: main: foo.x =
// BC-NEXT:   s4[ASCII, {{.*}}]: main: init
// BC-NEXT:   i5[ASCII, {{.*}}] #{{.*}}: alpha
// BC-NEXT:   i6[ASCII, {{.*}}] #{{.*}}: context
// BC-NEXT:   i7[ASCII, {{.*}}] #{{.*}}: x
// BC-NEXT:   i8[ASCII, {{.*}}] #{{.*}}: loadSegment
// BC-NEXT:   i9[ASCII, {{.*}}] #{{.*}}: print

// BC-LABEL: String count: 8
// BC-LABEL: Global String Table:
// BC-NEXT:   s0[ASCII, {{.*}}]: bar: init
// BC-NEXT:   s1[ASCII, {{.*}}]: cjs_module
// BC-NEXT:   s2[ASCII, {{.*}}]: foo: bar.y =
// BC-NEXT:   s3[ASCII, {{.*}}]: foo: init
// BC-NEXT:   s4[ASCII, {{.*}}]: global
// BC-NEXT:   i5[ASCII, {{.*}}] #{{.*}}: print
// BC-NEXT:   i6[ASCII, {{.*}}] #{{.*}}: x
// BC-NEXT:   i7[ASCII, {{.*}}] #{{.*}}: y
