/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-bytecode %s | %FileCheck %s --match-full-lines

// Inner HideSource can override outer ShowSource
function showSource1() {
  'show source';
  function hideSource() {
    'hide source';
  }
}

// Inner ShowSource cannot override outer HideSource/Sensitive
function sensitive() {
  'sensitive';
  function showSource2() {
    'show source';
  }
}

// CHECK-LABEL: Global String Table:
// CHECK-NEXT: s0[ASCII, 0..-1]:
// CHECK-NEXT: s1[ASCII, 0..91]: function showSource1() {\x0A  'show source';\x0A  function hideSource() {\x0A    'hide source';\x0A  }\x0A}
