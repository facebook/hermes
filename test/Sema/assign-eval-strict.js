/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes %s -dump-ir 2>&1 ) | %FileCheck --match-full-lines %s

"use strict";
eval = 0;
// CHECK: {{.*}}:[[@LINE-1]]:1: error: invalid assignment left-hand side
