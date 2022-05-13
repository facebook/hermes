/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fnc < %s

function thrower(){
  throw 11;
}

var x = 31;

try {
  thrower();
} catch (e){
  let y = e + 3;
  x += y;
}
