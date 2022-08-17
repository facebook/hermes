/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

print("setTimeout");
// CHECK-LABEL: setTimeout

setTimeout(function() {});
clearTimeout(4294967294);

var id = setTimeout(function() {
  print('In timeout');
});

var id2 = setTimeout(function() {
  print('In timeout2');
});

clearTimeout(id2);

// CHECK-NEXT: In timeout
// CHECK-NOT: In timeout2
