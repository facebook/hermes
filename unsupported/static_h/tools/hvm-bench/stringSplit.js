/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var USE_REGEXP = false;

  var numIter = 10000;
  var len = 1000;
  var s = 'aaaa01'.repeat(len);
  var separator = USE_REGEXP ? /[0-9]{2}/ : '01';

  for (var j = 0; j < numIter; j++) {
    s.split(separator);
  }

  print('done');
})();
