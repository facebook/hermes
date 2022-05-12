/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fnc < %s

var arr = [1,3,5,7,9];

var sum = 0;
for (let i = 0; i < 5; i++){
  sum += arr[i];
}
