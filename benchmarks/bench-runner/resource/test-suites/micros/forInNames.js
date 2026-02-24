/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  function foo(x) {
    var sum = 0;
    for (var i in x) {
      sum += x[i];
    }
    return sum;
  }
  var N = 1_000_000;
  var obj = {};
  for (var i = 0; i < 50; ++i) {
    obj['a' + i] = i;
  }
  var sum = 0;
  var start = Date.now();
  for (var i = 0; i < N; ++i) {
    sum += foo(obj);
  }
  var end = Date.now();
  print("Time:", end - start);

  return sum;
})();
