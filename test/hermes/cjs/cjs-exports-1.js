/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Werror -commonjs %S/cjs-exports-1.js %S/cjs-exports-2.js | %FileCheck --match-full-lines %s
// RUN: %hermes -Werror -O -fstatic-require -fstatic-builtins -commonjs %S/cjs-exports-1.js %S/cjs-exports-2.js | %FileCheck --match-full-lines %s
'use strict';

print('1: init');
// CHECK-LABEL: 1: init

// Ensure we don't get warnings from accessing built-in globals in a module.
print('1:', Object.is({}, {}));
// CHECK-NEXT: 1: false

var mod2 = require('./cjs-exports-2.js');
// CHECK-NEXT: 2: init

print('1: mod2.x =', mod2.x);
// CHECK-NEXT: 1: mod2.x = 3
