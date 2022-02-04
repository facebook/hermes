/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: true

print('foo: init');

var bar = require('../bar/cjs-subdir-bar.js');

function subFun() {
  print('foo: bar.y =', bar.y);
}
subFun();

exports.x = bar.y;
