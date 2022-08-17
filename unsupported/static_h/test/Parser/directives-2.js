/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -non-strict %s 2>&1 ) | %FileCheck --match-full-lines %s

// Make sure we scan directive prologues.

"use the force"
"use strict"
010
//CHECK: {{.*}}directives-2.js:14:1: error: Octal literals must use '0o' in strict mode
//CHECK-NEXT: 010
//CHECK-NEXT: ^~~
