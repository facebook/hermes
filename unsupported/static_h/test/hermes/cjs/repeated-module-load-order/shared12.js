/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: true

print('init shared12.js');

var counter = 0;

module.exports = function increment() {
  print('shared12.js counter = ' + ++counter);
};
