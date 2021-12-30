/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {
  var len = 10000;

  var a = [];
  for (var j = 0; j < len; j++) {
    a.unshift(j);
  }

  print('done');
})();
