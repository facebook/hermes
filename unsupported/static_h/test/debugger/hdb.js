/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --break-at-start %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

// CHECK: Break on script load in global: {{.*}}:12:1
print("Hello hdb!");
debugger;
print("Good bye hdb!");
// CHECK-NEXT: Hello hdb!
// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}:13:1
// CHECK-NEXT: Good bye hdb!
