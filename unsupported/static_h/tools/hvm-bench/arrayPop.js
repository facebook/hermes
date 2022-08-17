/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var numIter = 10000;
  var len = 1000;

  for (var i = 0; i < numIter; i++) {
    var a = Array(len);
    for (var j = 0; j < len; j++) {
      a.pop();
    }
  }

  print('done');
})();
