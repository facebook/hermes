/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines --check-prefix=TASK %s
// RUN: %hermes -Xmicrotask-queue %s | %FileCheck --match-full-lines --check-prefix=MICROTASK %s

print('promise jobs scheduled in tasks');
// CHECK-LABEL: promise jobs scheduled in tasks

setTimeout(_ => {
  print('task1')
  Promise.resolve().then(_ => print('promise in task1'))
  setTimeout(_ => print('task3'), 0);
}, 0);

setTimeout(_ => {
  print('task2')
  setTimeout(_ => print('task4'), 0);
  Promise.resolve().then(_ => print('promise in task2'))
}, 0);

// TASK: task1
// TASK-NEXT: task2
// TASK-NEXT: promise in task1
// TASK-NEXT: task3
// TASK-NEXT: task4
// TASK-NEXT: promise in task2

// MICROTASK: task1
// MICROTASK-NEXT: promise in task1
// MICROTASK-NEXT: task2
// MICROTASK-NEXT: promise in task2
// MICROTASK-NEXT: task3
// MICROTASK-NEXT: task4
