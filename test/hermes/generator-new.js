/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s
// TODO(T168592126) for now we don't run against shermes,
// since function calls are not checked.

print('generators');;
// CHECK-LABEL: generators

function* simple() {
  yield 1;
}

try {
  new simple();
  print('must throw');
} catch (e) {
  print('caught', e.name);
}
// CHECK-NEXT: caught TypeError
