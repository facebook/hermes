/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

var start = Date.now();

var numIter = 1000000;

for (var i = 0; i < numIter; i++) {
  String.fromCharCode(i);
}

print('done');
var end = Date.now();
print("Time: " + (end - start));
