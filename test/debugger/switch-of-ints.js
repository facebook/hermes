/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --break-at-start < %s.debug %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function intSwitch(s) {
  'noinline'
  switch (s) {
  case 0:
    return 0;
  case 1:
    return 1;
  case 2:
    return 2;
  case 3:
    return 3;
  case 4:
    return 4;
  case 5:
    return 5;
  case 6:
    return 6;
  case 7:
    return 7;
  case 8:
    return 8;
  case 9:
    return 9;
  }
  return 1000;
}

print(intSwitch(7));

// CHECK: Break on script load in global: {{.*}}:11:1
// CHECK-NEXT: Stepped to global: {{.*}}:38:1
// CHECK-NEXT: Stepped to intSwitch: {{.*}}:13:11
// CHECK-NEXT: Stepped to intSwitch: {{.*}}:29:5


