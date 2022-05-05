/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (%juno %s 2>&1 || true) | %FileCheck %s --match-full-lines

'use strict'
arguments = 0;

// CHECK-LABEL: {{.*}}:11:1: error: invalid assignment left-hand side
// CHECK-NEXT: 1 error(s), 0 warning(s)
