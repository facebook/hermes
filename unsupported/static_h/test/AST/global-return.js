/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes %s 2>&1 ) | %FileCheck --match-full-lines %s

return;
//CHECK: {{.*}}global-return.js:10:1: error: 'return' not in a function
//CHECK-NEXT: return;
//CHECK-NEXT: ^~~~~~~
