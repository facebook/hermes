/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheck --match-full-lines %s

var o = {};
Object.defineProperty(o, 'foo', { value: 'bar' });
print(o.foo)

// CHECK-LABEL: Bytecode File Information:
// CHECK:       String Kind Entry count: 2

// CHECK-LABEL: Global String Table:
// CHECK-NEXT:  s0[ASCII, {{.*}}]: bar
// CHECK-NEXT:  s1[ASCII, {{.*}}]: global
// CHECK-NEXT:  i2[ASCII, {{.*}}] #{{.*}}: Object
// CHECK-NEXT:  i3[ASCII, {{.*}}] #{{.*}}: defineProperty
// CHECK-NEXT:  i4[ASCII, {{.*}}] #{{.*}}: foo
// CHECK-NEXT:  i5[ASCII, {{.*}}] #{{.*}}: o
// CHECK-NEXT:  i6[ASCII, {{.*}}] #{{.*}}: print
// CHECK-NEXT:  i7[ASCII, {{.*}}] #{{.*}}: value
