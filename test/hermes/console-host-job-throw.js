/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xmicrotask-queue %s 2>&1 | %FileCheck --match-full-lines %s

print("ConsoleHost job throws");
// CHECK-LABEL: ConsoleHost job throws

var queueMicrotask = HermesInternal.enqueueJob;

queueMicrotask(_ => {
  print("job1");
});

queueMicrotask(_ => {
  print("job2");
  throw new Error('from job2')
});

queueMicrotask(_ => {
  print("job3");
});

try {
  queueMicrotask(_ => {
    print("job4");
    throw new Error('from job4')
  });
} catch (e) {
  print("uncatchable")
}

queueMicrotask(_ => {
  print("job5");
});

// CHECK-NEXT: job1
// CHECK-NEXT: job2
// CHECK-NEXT: Uncaught Error: from job2
// CHECK-NEXT:  at anonymous ({{.*}})

// CHECK-NEXT: job3
// CHECK-NEXT: job4
// CHECK-NEXT: Uncaught Error: from job4
// CHECK-NEXT:  at anonymous ({{.*}})

// CHECK-NEXT: job5
