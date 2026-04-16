/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

var start = Date.now();
(function() {
  var numIter = 200;
  var len = 10000;
  var a = new Int32Array(len);
  for (var i = 0; i < len; i++) {
    a[i] = i;
  }

  var sum = 0;

  for (var i = 0; i < numIter; i++) {
    for (var j = 0; j < len; j++) {
      sum += a.indexOf(j);
    }
  }

  print('done, sum =', sum);
})();
var end = Date.now();
print("Time: " + (end - start));
