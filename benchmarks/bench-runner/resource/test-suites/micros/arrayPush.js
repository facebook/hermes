/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

var start = Date.now();
(function () {
  var numIter = 1_000_000;
  var items = [];

  for (var i = 0; i < numIter; i++) {
    items.push(i);
  }
})();
var end = Date.now();
print('Time: ' + (end - start));
