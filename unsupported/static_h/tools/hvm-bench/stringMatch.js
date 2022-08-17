/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var USE_REGEXP = true;

  var source =
    'abcdefghijklmnopqrstuvwxyz'.repeat(10) +
    'he110' +
    'abcdefghijklmnopqrstuvwxyz'.repeat(10);
  var numIter = 300000;
  var searchVal = USE_REGEXP ? /[a-z]{2}[0-9]{3}/ : 'he110';

  for (var i = 0; i < numIter; i++) {
    source.match(searchVal);
  }

  print('done');
})();
