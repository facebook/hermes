/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -commonjs %S/cjs-throw-1.js %S/cjs-throw-2.js | %FileCheck --match-full-lines %s
// RUN: %hermes -commonjs -fstatic-require -fstatic-builtins -O %S/cjs-throw-1.js %S/cjs-throw-2.js | %FileCheck --match-full-lines %s

// cjs-throw-2 expects that x === 5, and throws otherwise.

exports.x = 3;
try { require('./cjs-throw-2.js'); } catch (e) { print('caught', e.message); }
// CHECK: caught INVALID X

exports.x = 4;
try { require('./cjs-throw-2.js'); } catch (e) { print('caught', e.message); }
// CHECK: caught INVALID X

exports.x = 5;

var y = require('./cjs-throw-2.js').y;
print(y);
// CHECK: 42

var y = require('./cjs-throw-2.js').y;
print(y);
// CHECK: 42

// After require is successful, don't run the code in the module again.
exports.x = 1000;
var y = require('./cjs-throw-2.js').y;
print(y);
// CHECK: 42
