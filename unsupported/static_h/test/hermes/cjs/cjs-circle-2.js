/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: true

print('2: init');

module.exports = {x: 1};

var mod3 = require('./cjs-circle-3.js');

module.exports = {y: 2};
print('2: mod3.z =', mod3.z);
module.exports.z = mod3.z;
