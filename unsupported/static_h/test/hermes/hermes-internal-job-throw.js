/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xhermes-internal-test-methods=true %s | %FileCheck --match-full-lines %s

print("HermesInternal job throws");
// CHECK-LABEL: HermesInternal job throws

HermesInternal.enqueueJob(_ => {
  print("job1");
});

HermesInternal.enqueueJob(_ => {
  print("job2");
  throw new Error('from job2')
});

HermesInternal.enqueueJob(_ => {
  print("job3");
});

HermesInternal.enqueueJob(_ => {
  print("job4");
  throw new Error('from job4')
});

HermesInternal.enqueueJob(_ => {
  print("job5");
});

// Starts draining, then stops when job2 throws.
print('drainJobs1');
try {
  HermesInternal.drainJobs?.();
} catch (e) {
  print(e);
}
// CHECK-NEXT: drainJobs1
// CHECK-NEXT: job1
// CHECK-NEXT: job2
// CHECK-NEXT: Error: from job2

// Resumes draining, then stop again when job4 throws.
print('drainJobs2');
try {
  HermesInternal.drainJobs?.();
} catch (e) {
  print(e);
}
// CHECK-NEXT: drainJobs2
// CHECK-NEXT: job3
// CHECK-NEXT: job4
// CHECK-NEXT: Error: from job4

// Resumes draining, and successfully empty the queue.
print('drainJobs3');
try {
  HermesInternal.drainJobs?.();
} catch (e) {
  print(e);
}
// CHECK-NEXT: drainJobs3
// CHECK-NEXT: job5
