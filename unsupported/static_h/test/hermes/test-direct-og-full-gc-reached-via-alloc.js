/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -gc-alloc-young=false -gc-revert-to-yg-at-tti=true -gc-init-heap=12M -gc-sanitize-handles=0 %s

var rootArray = [];

function makeArray(n) {
  var a = [];
  for (k = 0; k < n; k++) {
    a[k] = {a: 1, b: "s"};
  }
  return a;
}

var tmpArray;
function doSomeAlloc(n) {
  for (var j = 0; j < n; j++) {
    tmpArray = makeArray(100);
    rootArray[j] = makeArray(100);
  }
}

for (var i = 0; i < 10; ++i) {
  doSomeAlloc(100);
}
