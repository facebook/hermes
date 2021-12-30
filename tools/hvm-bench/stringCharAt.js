/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var numIter = 2000;
  var len = 10000;
  var s = 'x'.repeat(len);

  for (var j = 0; j < numIter; j++) {
    for (var i = 0; i < len; i++) {
      s.charAt(i);
    }
  }

  print('done');
})();
