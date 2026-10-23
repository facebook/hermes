/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s

// The array fast path must observe index-like properties added to a
// dictionary-mode object in Array.prototype's parent chain.

(function () {
  // Put Object.prototype into dictionary mode.
  Object.prototype.tmp = 1;
  delete Object.prototype.tmp;

  var arr = [1, , 3];
  print(arr.includes(5), arr.join(','));
  // CHECK: false 1,,3

  Object.prototype[1] = 5;
  print(arr.includes(5), arr.join(','));
  // CHECK-NEXT: true 1,5,3
  delete Object.prototype[1];
})();
