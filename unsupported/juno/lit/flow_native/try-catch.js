/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fn_dir/run_fnc.sh %fnc %s %t && %t | %FileCheck %s --match-full-lines

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
print(x);
// CHECK: 45.000000
