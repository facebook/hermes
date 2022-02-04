/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb < %s.debug %s --break-after 1 | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print("Testing async break");

var x = 1;
// Behold the power of hdb, as it completes this loop.
while (typeof done === 'undefined')
  x += 1;
print("Done!");

// CHECK: Testing async break
// CHECK-NEXT: Interrupted in {{.*}}
// CHECK-NEXT: {{[0-9]+}}
// CHECK-NEXT: Done!
