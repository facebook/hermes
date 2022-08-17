/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var numIter = 1000000;
  var source =
    'abcdefghijklmnopqrstuvwxyz'.repeat(10) +
    'hello' +
    'abcdefghijklmnopqrstuvwxyz'.repeat(10);

  for (var i = 0; i < numIter; i++) {
    source.includes('hello');
  }
  for (var i = 0; i < numIter; i++) {
    source.includes('haaal');
  }

  print('done');
})();
