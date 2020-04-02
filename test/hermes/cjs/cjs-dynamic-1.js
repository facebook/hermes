/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -commonjs %S/cjs-dynamic-1.js %S/cjs-dynamic-2.js | %FileCheck --match-full-lines %s
// RUN: %hermes -commonjs -fstatic-require -fstatic-builtins -O %S/cjs-dynamic-1.js %S/cjs-dynamic-2.js | %FileCheck --match-full-lines %s --check-prefix=STATIC

print(require('./cjs-dynamic-2.js'))
// CHECK: 3
// STATIC: 3

print(require('./cjs-dynamic-' + (2 + (Math.random() * 0)) + '.js'));
// CHECK: 3
// STATIC: 3
