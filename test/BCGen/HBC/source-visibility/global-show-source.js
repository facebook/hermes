/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-bytecode %s | %FileCheck %s --match-full-lines

'show source';

function foo(x) { return x }
function bar(y) { return y }
function baz(z) { return z }

// HideSource can override ShowSource
function hideSource (x) {
  'hide source';
  return x;
}

// CHECK-LABEL: Function source count: 4

// CHECK-LABEL: Global String Table:
// CHECK-NEXT: s0[ASCII, 0..-1]: 
// CHECK-NEXT: s1[ASCII, 0..27]: function bar(y) { return y }
// CHECK-NEXT: s2[ASCII, 28..55]: function baz(z) { return z }
// CHECK-NEXT: s3[ASCII, 56..83]: function foo(x) { return x }

// CHECK-LABEL: Function Source Table:
// CHECK-NEXT:   Function ID 1 -> s3
// CHECK-NEXT:   Function ID 2 -> s1
// CHECK-NEXT:   Function ID 3 -> s2
// CHECK-NEXT:   Function ID 4 -> s0
