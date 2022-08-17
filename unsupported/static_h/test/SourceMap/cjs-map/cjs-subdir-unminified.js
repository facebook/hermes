/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

var mod2 = require('cjs-subdir-2.min.js');
function run() {
  try {
    mod2.throwError();
  } catch (e) {
    print(e.stack);
  }
}

exports.run = run;
