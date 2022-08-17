/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines --check-prefix=TASK %s
// RUN: %hermes -Xmicrotask-queue %s | %FileCheck --match-full-lines --check-prefix=MICROTASK %s

print('promise jobs scheduled in scripts');
// CHECK-LABEL: promise jobs scheduled in scripts

setTimeout(_ => print('setTimeout1'), 0);

Promise.resolve()
  .then(_ => print('promise1'))
  .then(_ => {
    print('promise2');

    // Promise jobs enqueued during the current draining.
    Promise.resolve()
      .then(_ => {
        print('promise3')
        setTimeout(_ => print('setTimeout3'), 0);
      }).then(_ => print("promise4"));
  });

setTimeout(_ => print('setTimeout2'), 0);

// TASK: setTimeout1
// TASK-NEXT: promise1
// TASK-NEXT: setTimeout2
// TASK-NEXT: promise2
// TASK-NEXT: promise3
// TASK-NEXT: setTimeout3
// TASK-NEXT: promise4

// MICROTASK: promise1
// MICROTASK-NEXT: promise2
// MICROTASK-NEXT: promise3
// MICROTASK-NEXT: promise4
// MICROTASK-NEXT: setTimeout1
// MICROTASK-NEXT: setTimeout2
// MICROTASK-NEXT: setTimeout3
