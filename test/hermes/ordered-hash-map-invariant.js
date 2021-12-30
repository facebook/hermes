/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -gc-sanitize-handles=0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -emit-binary -out %t.hbc %s && %hermes -gc-sanitize-handles=0 %t.hbc | %FileCheck --match-full-lines %s
// REQUIRES: !slow_debug

// At one point, growing a Set (or other user of OrderedHashMap) to a
// large size caused an assertion to fire.  Test that we can do so
// successfully.
var s = new Set();
for (var i = 0; i < 500000; i++) {
  s.add(i);
}
print('completed');
//CHECK:completed
