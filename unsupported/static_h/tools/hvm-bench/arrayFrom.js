/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var USE_ITERABLE_ITEMS = true;
  var USE_ARRAY = true;
  var USE_MAPFN = true;
  var USE_THISARG = true;

  var numIter = 800;
  var len = 5000;

  var items = USE_ITERABLE_ITEMS ? Array(len) : {};
  for (var i = 0; i < len; i++) {
    items[i] = i;
  }

  var mapfn, thisArg;

  if (USE_MAPFN) {
    mapfn = function(val, index) {
      return val + 1;
    };
  }

  if (USE_THISARG) {
    thisArg = items;
  }

  if (USE_ARRAY) {
    for (var i = 0; i < numIter; i++) {
      Array.from(items, mapfn, thisArg);
    }
  } else {
    for (var i = 0; i < numIter; i++) {
      Array.from.call(Object, items, mapfn, thisArg);
    }
  }

  print('done');
})();
