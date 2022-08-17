/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var numIter = 300000;
  var len = 1000;
  var rx = RegExp('a'.repeat(len), 'gimuy');

  for (var i = 0; i < numIter; i++) {
    rx.toString();
  }

  print('done');
})();
