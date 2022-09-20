/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// Ensure that handles do not accumulate across bound calls.

var a = function() {
    print(this);
}
for(i=0; i < 100; i++) {
  a = a.bind("callBound");
}
a();
//CHECK: callBound

var b = String;
for(i=0; i < 100; i++) {
  b = b.bind(10, "constructBound");
}
print(new b());
//CHECK-NEXT: constructBound
