/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var numIter = 50000;
  var len = 8000;
  var s = 'str';

  for (var j = 0; j < numIter; j++) {
    s.padEnd(len, 'x');
  }

  print('done');
})();
