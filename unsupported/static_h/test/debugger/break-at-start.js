/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug --break-after 0 | %FileCheck --match-full-lines %s
// REQUIRES: debugger

0;

// CHECK: Interrupted in global: {{.*}}
// CHECK-NEXT: Hello from the debugger
