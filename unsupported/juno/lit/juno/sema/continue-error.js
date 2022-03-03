/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (%juno %s --gen-sema 2>&1 || true) | %FileCheck %s --match-full-lines

switch (1) {
  default:
  continue;
}
// CHECK: {{.*}}:12:3: error: 'continue' not within a loop or switch

x: {
  continue;
}
// CHECK-NEXT: {{.*}}:17:3: error: 'continue' not within a loop or switch

a: {
  while(1) {
    continue a;
  }
}
// CHECK-NEXT: {{.*}}:23:14: error: 'continue' label 'a' is not a loop label

while(1) {
  continue b;
}
// CHECK-NEXT: {{.*}}:29:12: error: label 'b' is not defined
