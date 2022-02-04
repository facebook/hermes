/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xhermes-internal-test-methods=true %s | %FileCheck --match-full-lines %s

print("HermesInternal job queue operations");
// CHECK-LABEL: HermesInternal job queue operations

HermesInternal.enqueueJob(_ => print("job1"));
HermesInternal.enqueueJob(_ => print("job2"));

HermesInternal.enqueueJob(_ => {
  print("job3");
  HermesInternal.enqueueJob(_ => print("job5"));
});

HermesInternal.enqueueJob(_ => {
  print("job4");
  HermesInternal.enqueueJob(_ => {
    print("job6");
    HermesInternal.enqueueJob(_ => print("job7"));
  });
});

HermesInternal.drainJobs?.();
// CHECK-NEXT: job1
// CHECK-NEXT: job2
// CHECK-NEXT: job3
// CHECK-NEXT: job4
// CHECK-NEXT: job5
// CHECK-NEXT: job6
// CHECK-NEXT: job7
