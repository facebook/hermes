/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -gc-sanitize-handles=0 -gc-max-heap=12M %s | %FileCheck --match-full-lines %s

function testCompact() {
  print("testCompact");
//CHECK-LABEL: testCompact
  var s = new Set();
  for (var i = 0; i < 100000; ++i) {
    s.add(i);
    s.delete(i);
  }
  print(s.size);
//CHECK-NEXT: 0
}

testCompact();
