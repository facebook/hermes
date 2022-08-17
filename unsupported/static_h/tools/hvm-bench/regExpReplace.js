/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var USE_REPLACE_FUNC = false;
  function letterF() {
    return 'f';
  }

  var numIter = 2000;
  var len = 800;
  var re = /cd/g;
  var s = 'abcde'.repeat(len);
  var replaceVal = USE_REPLACE_FUNC ? letterF : 'f';

  var _SymbolReplace = Symbol.replace;
  for (var i = 0; i < numIter; i++) {
    re[_SymbolReplace](s, replaceVal);
  }

  print('done');
})();
