/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

function entryPoint() {
  helper();
}

function helper() {
  var s = "abc";
  var x = 1;
  var y;
  var z = false;
  throw new Error("exception is thrown");
}

entryPoint();
