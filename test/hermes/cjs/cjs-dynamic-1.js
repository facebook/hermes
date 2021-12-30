/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -commonjs %S/cjs-dynamic-1.js %S/cjs-dynamic-2.js | %FileCheck --match-full-lines %s
// RUN: %hermes -commonjs -fstatic-require -fstatic-builtins -O %S/cjs-dynamic-1.js %S/cjs-dynamic-2.js | %FileCheck --match-full-lines %s --check-prefix=STATIC
// RUN: %hermes -Wno-error=unresolved-static-require -commonjs -fstatic-require -fstatic-builtins -O %S/cjs-dynamic-1.js %S/cjs-dynamic-2.js 2>&1 | %FileCheck --match-full-lines %s --check-prefixes=WARN,CHECK
// RUN: ( ! %hermes -Werror=unresolved-static-require -commonjs -fstatic-require -fstatic-builtins -O %S/cjs-dynamic-1.js %S/cjs-dynamic-2.js 2>&1 ) | %FileCheck --match-full-lines %s --check-prefix ERROR
// RUN: %hermes -Wno-unresolved-static-require -commonjs -fstatic-require -fstatic-builtins -O %S/cjs-dynamic-1.js %S/cjs-dynamic-2.js 2>&1 | %FileCheck --match-full-lines %s --check-prefixes=SILENCED,CHECK

// WARN: {{.*}}/cjs-dynamic-1.js:22:14: warning: require() argument cannot be coerced to constant string at compile time
// ERROR: {{.*}}/cjs-dynamic-1.js:22:14: error: require() argument cannot be coerced to constant string at compile time
// SILENCED-NOT: {{.*}} require() argument cannot be coerced to constant string at compile time

print(require('./cjs-dynamic-2.js'))
// CHECK: 3
// STATIC: 3

print(require('./cjs-dynamic-' + (2 + (Math.random() * 0)) + '.js'));
// CHECK: 3
// STATIC: 3
