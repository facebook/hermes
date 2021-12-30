/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: true

print('3: init');

var mod2 = require('./cjs-circle-2.js');

// Require mod2 while it's initializing, get current exported object.
print('3: mod2.x =', mod2.x);
print('3: mod2.y =', mod2.y);
exports.z = 42;
