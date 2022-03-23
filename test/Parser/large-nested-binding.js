/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: ( ! %hermes %s 2>&1 ) | %FileCheck --match-full-lines %s

var v2 = "[".repeat(100000);
// CHECK: {{.*}}Too many nested expressions/statements/declarations{{.*}}
var v5 = Function(v2,"return 0");
