/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=0 %s | %FileCheck --match-full-lines %s
// UNSUPPORTED: slow_debug || gc_malloc

try {
  var a = new String("foo");
  for (var j = 0; true; j++) {
    a[j] = undefined;
  }
} catch (e) {
  print(e.message);
}

//CHECK: Property storage exceeds {{.*}} properties
