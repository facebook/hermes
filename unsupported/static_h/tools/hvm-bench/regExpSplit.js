/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var numIter = 5000;
  var len = 1000;
  var s = 'aaa001'.repeat(len);
  var rx = /[0-9]+/;
  var _SymbolSplit = Symbol.split;

  for (var i = 0; i < numIter; i++) {
    rx[_SymbolSplit](s);
  }

  print('done');
})();
