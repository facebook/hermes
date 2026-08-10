/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xmicrotask-queue %s | %FileCheck --match-full-lines %s
// RUN: %hermes %s | %FileCheck --match-full-lines %s

print(typeof queueMicrotask);
// CHECK-LABEL: function

try {
  queueMicrotask(1);
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: TypeError

queueMicrotask(function () {
  print('microtask1');
  queueMicrotask(function () {
    print('microtask3');
  });
});
queueMicrotask(function () {
  print('microtask2');
});
print('sync');
// CHECK-NEXT: sync
// CHECK-NEXT: microtask1
// CHECK-NEXT: microtask2
// CHECK-NEXT: microtask3
