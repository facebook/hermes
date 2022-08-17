/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var USE_ARRAY = true;

  var numIter = 6000;
  var len = 5000;

  var items = Array(len);
  for (var i = 0; i < len; i++) {
    items[i] = i;
  }

  var constructor = USE_ARRAY ? Array : Object;

  for (var i = 0; i < numIter; i++) {
    Array.of.apply(constructor, items);
  }
})();
