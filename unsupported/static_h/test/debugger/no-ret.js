/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: ASAN_OPTIONS=leak_check_at_exit=0 %hdb --break-at-start %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

// Turn off leak checks at exit because hdb calls exit while a Runtime is still alive.

 for (;;);

// CHECK: Break on script load in global: {{.*}}:13:2
// CHECK: Stepped to global: {{.*}}:13:2
// CHECK: Stepped to global: {{.*}}:13:2
