/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

exports.throwError = function mod2fun() {
  require('cjs-subdir-shared.js').immediatelyInvoke(function mod2Inner() {
    throw new Error('ERROR_FOR_TESTING');
  });
  return;
}
