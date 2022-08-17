/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: true

var m = {
  doPrint() {
    print('in export12.js doPrint()');
  },
};

for (var p in m) {
  if (!exports.hasOwnProperty(p)) exports[p] = m[p];
}
