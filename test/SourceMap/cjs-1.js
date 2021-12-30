/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

var mod2 = require('./cjs-2.js');

function throws1() {
  mod2.throws2();
}

try {
  throws1();
} catch(err) {
  print(err.stack);
}
