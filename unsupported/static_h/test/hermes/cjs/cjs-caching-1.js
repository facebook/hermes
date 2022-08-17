/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -commonjs %S/cjs-caching-1.js %S/cjs-caching-2.js %S/cjs-caching-3.js | %FileCheck --match-full-lines %s
// RUN: %hermes -O -fstatic-require -commonjs %S/cjs-caching-1.js %S/cjs-caching-2.js %S/cjs-caching-3.js | %FileCheck --match-full-lines %s
// RUN: %hermes -O -commonjs %S/cjs-caching-1.js %S/cjs-caching-2.js %S/cjs-caching-3.js | %FileCheck --match-full-lines %s

const copy1 = require('./cjs-caching-2.js');
copy1.recreateExports();
const copy2 = require('./cjs-caching-2.js');
print('With no dependency cycle:');
print(copy1 === copy2 ? 'Copies are equal' : 'Copies are different');

// CHECK: With a dependency cycle:
// CHECK-NEXT: Copies are different
// CHECK-NEXT: With no dependency cycle:
// CHECK-NEXT: Copies are equal
