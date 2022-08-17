/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: ( ! %hdb %s < %s.debug 2>&1 ) | %FileCheck --match-full-lines %s
// REQUIRES: debugger

debugger;
throw 'err(asdf)';

// CHECK: Break on 'debugger' statement in global: {{.*}}:11:1
// CHECK-NEXT: Set pauseOnThrow: all errors
// CHECK-NEXT: Stepped to global: {{.*}}:12:1
// CHECK-NEXT: Break on exception in global: {{.*}}:12:1
// CHECK-NEXT: JavaScript terminated via uncaught exception: err(asdf)
