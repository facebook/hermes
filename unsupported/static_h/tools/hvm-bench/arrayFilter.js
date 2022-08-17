/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var USE_THISARG = false;

  var numIter = 2000;
  var len = 10000;
  var thisArg = undefined;
  var a = Array(len);
  for (var i = 0; i < len; i++) {
    a[i] = i;
  }

  if (USE_THISARG) thisArg = a;

  function isEven(val) {
    return val % 2 === 0;
  }

  for (var i = 0; i < numIter; i++) {
    a.filter(isEven, thisArg);
  }

  print('done');
})();
