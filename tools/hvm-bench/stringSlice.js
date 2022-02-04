/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var numIter = 8000;
  var len = 1000;
  var s = 'x'.repeat(len);

  for (var i = 0; i < numIter; i++) {
    for (var j = 0; j < len / 2; j++) {
      s.slice(j, -j);
    }
  }

  print('done');
})();
