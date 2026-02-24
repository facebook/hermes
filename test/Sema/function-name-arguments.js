/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes %s -dump-ir 2>&1 ) | %FileCheck --match-full-lines %s

function arguments() { 'use strict'; }
// CHECK: {{.*}}:[[@LINE-1]]:10: error: cannot declare 'arguments' in strict mode

(function arguments() { 'use strict'; })
// CHECK: {{.*}}:[[@LINE-1]]:11: error: cannot declare 'arguments' in strict mode

function eval() {'use strict'; }
// CHECK: {{.*}}:[[@LINE-1]]:10: error: cannot declare 'eval' in strict mode
