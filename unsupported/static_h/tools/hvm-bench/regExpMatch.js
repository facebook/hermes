/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

var numIter = 4000;
var len = 1000;
var s = 'abcd'.repeat(len);
var rx = /bcd/g;

var _SymbolMatch = Symbol.match;
for (var i = 0; i < numIter; i++) {
  rx[_SymbolMatch](s);
}

print('done');
