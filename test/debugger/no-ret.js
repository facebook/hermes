/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --break-at-start %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

 for (;;);

// CHECK: Break on script load in global: {{.*}}:11:2
// CHECK: Stepped to global: {{.*}}:11:2
// CHECK: Stepped to global: {{.*}}:11:2
