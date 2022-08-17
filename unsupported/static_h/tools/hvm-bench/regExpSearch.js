/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var numIter = 200000;
  var len = 2000;
  var s = 'abcd'.repeat(len);
  var rx = /d.+d/g;

  var _SymbolSearch = Symbol.search;
  for (var i = 0; i < numIter; i++) {
    rx[_SymbolSearch](s);
  }

  print('done');
})();
