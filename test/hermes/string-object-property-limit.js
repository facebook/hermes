/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// REQUIRES: !slow_debug

try {
  var a = new String("foo");
  for (var j = 0; true; j++) {
    a[j] = {};
  }
} catch (e) {
  print(e.message);
}

//CHECK: Property storage exceeds {{.*}} properties
