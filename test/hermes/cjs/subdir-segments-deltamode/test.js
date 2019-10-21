/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -commonjs -fstatic-require -fstatic-builtins %S/update/ -emit-binary -out %T/test.hbc && %hermes -O -commonjs -fstatic-require -fstatic-builtins %S/update/ -base-bytecode %T/ -dump-bytecode | %FileCheck --match-full-lines %s -check-prefix BC-NO-CHANGE
// RUN: %hermes -O -commonjs -fstatic-require -fstatic-builtins %S/base/ -emit-binary -out %T/test.hbc && %hermes -O -commonjs -fstatic-require -fstatic-builtins %S/update/ -base-bytecode %T/ -dump-bytecode | %FileCheck --match-full-lines %s -check-prefix BC-DELTA

// BC-NO-CHANGE-LABEL: String count: 10
// BC-NO-CHANGE-LABEL: Global String Table:
// BC-NO-CHANGE-NEXT:   s0[ASCII, {{.*}}]: 2: init
// BC-NO-CHANGE-NEXT:   s1[ASCII, {{.*}}]: cjs_module
// BC-NO-CHANGE-NEXT:   s2[ASCII, {{.*}}]: global
// BC-NO-CHANGE-NEXT:   s3[ASCII, {{.*}}]: main: foo.x =
// BC-NO-CHANGE-NEXT:   s4[ASCII, {{.*}}]: main: init
// BC-NO-CHANGE-NEXT:   i5[ASCII, {{.*}}] #{{.*}}: alpha
// BC-NO-CHANGE-NEXT:   i6[ASCII, {{.*}}] #{{.*}}: loadSegment
// BC-NO-CHANGE-NEXT:   i7[ASCII, {{.*}}] #{{.*}}: x
// BC-NO-CHANGE-NEXT:   p8[ASCII, {{.*}}] @{{.*}}: context
// BC-NO-CHANGE-NEXT:   p9[ASCII, {{.*}}] @{{.*}}: print

// BC-NO-CHANGE-LABEL: String count: 8
// BC-NO-CHANGE-LABEL: Global String Table:
// BC-NO-CHANGE-NEXT:   s0[ASCII, {{.*}}]: bar: init
// BC-NO-CHANGE-NEXT:   s1[ASCII, {{.*}}]: cjs_module
// BC-NO-CHANGE-NEXT:   s2[ASCII, {{.*}}]: foo: bar.y =
// BC-NO-CHANGE-NEXT:   s3[ASCII, {{.*}}]: foo: init
// BC-NO-CHANGE-NEXT:   s4[ASCII, {{.*}}]: global
// BC-NO-CHANGE-NEXT:   i5[ASCII, {{.*}}] #{{.*}}: x
// BC-NO-CHANGE-NEXT:   i6[ASCII, {{.*}}] #{{.*}}: y
// BC-NO-CHANGE-NEXT:   p7[ASCII, {{.*}}] @{{.*}}: print

// BC-DELTA-LABEL: String count: 11
// BC-DELTA-LABEL: Global String Table:
// BC-DELTA-NEXT:   s0[ASCII, {{.*}}]: 2: init
// BC-DELTA-NEXT:   s1[ASCII, {{.*}}]: cjs_module
// BC-DELTA-NEXT:   s2[ASCII, {{.*}}]: global
// BC-DELTA-NEXT:   s3[ASCII, {{.*}}]: main: init
// BC-DELTA-NEXT:   s4[ASCII, {{.*}}]: main_base: foo.x =
// BC-DELTA-NEXT:   i5[ASCII, {{.*}}] #{{.*}}: alpha
// BC-DELTA-NEXT:   i6[ASCII, {{.*}}] #{{.*}}: loadSegment
// BC-DELTA-NEXT:   i7[ASCII, {{.*}}] #{{.*}}: x
// BC-DELTA-NEXT:   p8[ASCII, {{.*}}] @{{.*}}: context
// BC-DELTA-NEXT:   p9[ASCII, {{.*}}] @{{.*}}: print
// BC-DELTA-NEXT:   s10[ASCII, {{.*}}]: main: foo.x =

// BC-DELTA-LABEL: String count: 9
// BC-DELTA-LABEL: Global String Table:
// BC-DELTA-NEXT:   s0[ASCII, {{.*}}]: bar: init
// BC-DELTA-NEXT:   s1[ASCII, {{.*}}]: cjs_module
// BC-DELTA-NEXT:   s2[ASCII, {{.*}}]: foo: init
// BC-DELTA-NEXT:   s3[ASCII, {{.*}}]: foo_base: bar.y =
// BC-DELTA-NEXT:   s4[ASCII, {{.*}}]: global
// BC-DELTA-NEXT:   i5[ASCII, {{.*}}] #{{.*}}: x
// BC-DELTA-NEXT:   i6[ASCII, {{.*}}] #{{.*}}: y
// BC-DELTA-NEXT:   p7[ASCII, {{.*}}] @{{.*}}: print
// BC-DELTA-NEXT:   s8[ASCII, {{.*}}]: foo: bar.y =

