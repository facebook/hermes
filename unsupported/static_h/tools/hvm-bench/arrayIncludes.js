/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var numIter = 20000;
  var len = 100;
  var a = Array(len);
  for (var i = 0; i < len; i++) {
    a[i] = i;
  }

  for (var i = 0; i < numIter; i++) {
    for (var j = 0; j < len; j++) {
      a.includes(j);
    }
  }

  print('done');
})();
