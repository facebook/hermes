/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-bytecode %s | %FileCheck %s --match-full-lines

'hide source';

function foo(x) { return x }
function bar(y) { return y }
function baz(z) { return z }

// ShowSource cannot override HideSource.
function hideSource (x) {
  'show source';
  return x;
}

// CHECK-LABEL: Function source count: 4

// CHECK-LABEL: Global String Table:
// CHECK-NEXT: s0[ASCII, 0..-1]: 

// CHECK-LABEL: Function Source Table:
// CHECK-NEXT:   Function ID 1 -> s0
// CHECK-NEXT:   Function ID 2 -> s0
// CHECK-NEXT:   Function ID 3 -> s0
// CHECK-NEXT:   Function ID 4 -> s0
