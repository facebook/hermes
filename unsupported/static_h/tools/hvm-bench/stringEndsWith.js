/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var source = 'abcdefghijklmnopqrstuvwxyz';
  var substr = 'xyz';
  var numIter = 4000000;

  for (var i = 0; i < numIter; i++) {
    source.endsWith(substr);
    source.endsWith(substr, 20);
  }

  print('done');
})();
