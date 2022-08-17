/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-bytecode %s | %FileCheck %s --match-full-lines

function default1(x) {
  return x;
}

function showSource(x) {
  'show source';
  return x;
}

function hideSource(x) {
  'hide source';
  return x;
}

function sensitive(x) {
  'sensitive';
  return x;
}

function showSource2(y) {
  'show source';
  return y;
}

function default2(y) {
  return y;
}

// CHECK-LABEL: Function source count: 4

// CHECK-LABEL: Global String Table:
// CHECK-NEXT: s0[ASCII, 0..-1]: 
// CHECK-NEXT: s1[ASCII, 0..54]: function showSource(x) {\x0A  'show source';\x0A  return x;\x0A}
// CHECK-NEXT: s2[ASCII, 55..110]: function showSource2(y) {\x0A  'show source';\x0A  return y;\x0A}

// CHECK-LABEL: Function Source Table:
// CHECK-NEXT:   Function ID 2 -> s1
// CHECK-NEXT:   Function ID 3 -> s0
// CHECK-NEXT:   Function ID 4 -> s0
// CHECK-NEXT:   Function ID 5 -> s2
