/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fnc < %s

var arr = [1,2,3,4,5];

arr[10] = 50;

print(arr[0] + arr[2] + arr[4] + arr[10]);
