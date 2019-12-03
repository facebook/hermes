/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print('nullish coalescing');
// CHECK-LABEL: nullish coalescing

print(undefined ?? undefined);
// CHECK-NEXT: undefined
print(undefined ?? null);
// CHECK-NEXT: null
print(undefined ?? 3);
// CHECK-NEXT: 3
print(null ?? 4);
// CHECK-NEXT: 4
print(3 ?? undefined);
// CHECK-NEXT: 3
print(3 ?? 4);
// CHECK-NEXT: 3

function foo() {
  print('called foo');
  return 42;
}

print(3 ?? foo());
// CHECK-NEXT: 3
print(undefined ?? foo());
// CHECK-NEXT: called foo
// CHECK-NEXT: 42
