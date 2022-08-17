/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var source =
    'abcdefghijklmnopqrstuvwxyz'.repeat(10) +
    'hello' +
    'abcdefghijklmnopqrstuvwxyz'.repeat(10);
  var numIter = 1000000;

  for (var i = 0; i < numIter; i++) {
    source.indexOf('hello');
  }

  for (var i = 0; i < numIter; i++) {
    source.indexOf('haaal');
  }

  print('done');
})();
