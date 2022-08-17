/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -emit-binary -out %t.1.hbc %s && %hermesc -emit-binary -out %t.2.hbc %s && %hbc-diff %t.1.hbc %t.2.hbc | %FileCheck --match-full-lines %s

print('Diff this!');

// CHECK-LABEL: Increase from {{.*}}.1.hbc to {{.*}}.2.hbc:
// CHECK-NEXT: Total: 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: Function headers: 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: Small string table: 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: Overflow string table: 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: String storage: 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: Array buffer: 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: Object key buffer: 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: Object value buffer: 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: Regexp table: 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: Regexp storage: 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: CommonJS module table: 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: CommonJS module table (static): 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: Function bodies: 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: Function info: 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: Debug info: 0 B  ({{.*}} -> {{.*}})
// CHECK-NEXT: 0 of 1 functions seem new. Largest new sizes:
