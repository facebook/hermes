/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes %s 2>&1 ) | %FileCheck --match-full-lines %s

"use strict";

delete a;
//CHECK: {{.*}}delete-variable-strict.js:12:1: error: 'delete' of a variable is not allowed in strict mode
//CHECK-NEXT: delete a;
//CHECK-NEXT: ^~~~~~~~
