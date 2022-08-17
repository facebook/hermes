/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --break-at-start < %s.debug %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

var a = 1;
var b = a + 3;
var c = a * b;

// CHECK: Break on script load in global: {{.*}}:11:1
// CHECK: Set breakpoint 1 at {{.*}}:12:9
// CHECK: Continuing execution
// CHECK: Break on breakpoint 1 in global: {{.*}}:12:9
// CHECK: Stepped to global: {{.*}}:13:9
