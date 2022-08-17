/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  USE_REPLACE_FUNC = false;
  function letterF() {
    return 'f';
  }

  var numIter = 500000;
  var len = 1000;
  var s = 'a'.repeat(len) + 'b';
  var replaceVal = USE_REPLACE_FUNC ? letterF : 'f';

  for (var i = 0; i < numIter; i++) {
    s.replace('b', replaceVal);
  }

  print('done');
})();
