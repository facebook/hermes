/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: true

print('foo: init');

var bar = require('../bar/cjs-subdir-bar.js');
print('foo: bar.y =', bar.y);

var bar = require('bar');
print('foo: bar.y =', bar.y);

exports.x = bar.y;
