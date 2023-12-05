/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s

/// This tests the behaviour of the interpreter when the same register is
/// allocated for both the operand and result of IteratorBegin. A correct
/// implementation should ensure that the register holds the result value after
/// the instruction has executed.
function foo() {
  return {
    [Symbol.iterator]() {
      return {
        return() {
          print('RETURN');
          return {}
        },
        next() {
          print('next');
          return { done: true, value: undefined };
        }
      }
    }
  }
}
function main() {
  const iteratorVar = foo();
  let [] = iteratorVar;
  for (const i of iteratorVar) {}
  print(2);
  for (const i of iteratorVar) {}
  for (const i of iteratorVar) {}
  for (const i of iteratorVar) {}
  for (const i of iteratorVar) {}
  for (const i of iteratorVar) {}
  for (const i of iteratorVar) {}
  for (const i of iteratorVar) {}
  for (const i of iteratorVar) {}
  for (const i of iteratorVar) {}
  for (const i of iteratorVar) {}
}
main();

// CHECK: RETURN
// CHECK-NEXT: next
// CHECK-NEXT: 2
// CHECK-NEXT: next
// CHECK-NEXT: next
// CHECK-NEXT: next
// CHECK-NEXT: next
// CHECK-NEXT: next
// CHECK-NEXT: next
// CHECK-NEXT: next
// CHECK-NEXT: next
// CHECK-NEXT: next
// CHECK-NEXT: next
