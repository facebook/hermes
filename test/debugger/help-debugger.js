/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

debugger;

// CHECK: Break on 'debugger' statement in global:{{.*}}
// CHECK: These hdb commands are defined internally. {{.*}}
// CHECK: Modifies selected frame {{.*}}
// CHECK: Not a valid command. {{.*}}
