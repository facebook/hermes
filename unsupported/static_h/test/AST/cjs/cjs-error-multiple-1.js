/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -commonjs %S/cjs-error-multiple-1.js %S/cjs-error-multiple-2.js 2>&1 ) | %FileCheck %s

require('./cjs-error-multiple-2.js');

// CHECK: {{.*}}cjs-error-multiple-2.js:9:5: error: 'identifier' expected in declaration
// CHECK-NEXT: var = 3;
// CHECK-NEXT: ~~~~^
