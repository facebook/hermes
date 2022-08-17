/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -O %s 2>&1) | %FileCheck --match-full-lines %s

print("setTimeout are flushed after throw");
// CHECK-LABEL: setTimeout are flushed after throw

setTimeout(function() {
  print("timer task flushed");
  throw new Error("double throw from timer");
});

throw new Error("throw from main script");

// CHECK-NEXT: Uncaught Error: throw from main script
// CHECK-LABEL: timer task flushed
// CHECK-NEXT: Uncaught Error: double throw from timer
