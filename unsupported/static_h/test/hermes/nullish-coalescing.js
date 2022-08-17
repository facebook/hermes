/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

print('\nside effects');
// CHECK-LABEL: side effects
print(3 ?? foo());
// CHECK-NEXT: 3
print(undefined ?? foo());
// CHECK-NEXT: called foo
// CHECK-NEXT: 42
print(foo() ?? foo() ? 1 : 2);
// CHECK-NEXT: called foo
// CHECK-NEXT: 1

print('\nconditional');
// CHECK-LABEL: conditional
print(undefined ?? true ? 1 : 2);
// CHECK-NEXT: 1
print(undefined ?? false ? 1 : 2);
// CHECK-NEXT: 2
print(true ?? foo() ? 1 : 2);
// CHECK-NEXT: 1
print(false ?? foo() ? 1 : 2);
// CHECK-NEXT: 2
print((1, undefined) ?? true ? 1 : 2);
// CHECK-NEXT: 1
print((1, undefined) ?? false ? 1 : 2);
// CHECK-NEXT: 2
print((undefined ?? true) ?? false ? 1 : 2);
// CHECK-NEXT: 1
print((undefined ?? false) ?? false ? 1 : 2);
// CHECK-NEXT: 2
print((true ?? false) ?? false ? 1 : 2);
// CHECK-NEXT: 1
print((false ?? true) ?? false ? 1 : 2);
// CHECK-NEXT: 2
print((true && true) ?? true ? 1 : 2);
// CHECK-NEXT: 1
print((true && false) ?? true ? 1 : 2);
// CHECK-NEXT: 2
print((true && undefined) ?? true ? 1 : 2);
// CHECK-NEXT: 1
print((true && undefined) ?? false ? 1 : 2);
// CHECK-NEXT: 2
print((false || true) ?? true ? 1 : 2);
// CHECK-NEXT: 1
print((false || false) ?? true ? 1 : 2);
// CHECK-NEXT: 2
print((false || undefined) ?? true ? 1 : 2);
// CHECK-NEXT: 1
print((false || undefined) ?? false ? 1 : 2);
// CHECK-NEXT: 2
