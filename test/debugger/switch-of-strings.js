/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --break-at-start < %s.debug %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function stringSwitch(s) {
  'noinline'
  switch (s) {
  case "l0":
    return 0;
  case "l1":
    return 1;
  case "l2":
    return 2;
  case "l3":
    return 3;
  case "l4":
    return 4;
  case "l5":
    return 5;
  case "l6":
    return 6;
  case "l7":
    return 7;
  case "l8":
    return 8;
  case "l9":
    return 9;
  }
  return 1000;
}

print(stringSwitch("l1"));

// CHECK: Break on script load in global: {{.*}}:11:1
// CHECK-NEXT: Stepped to global: {{.*}}:38:1
// CHECK-NEXT: Stepped to stringSwitch: {{.*}}:13:11
// CHECK-NEXT: Stepped to stringSwitch: {{.*}}:17:5


