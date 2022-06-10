/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fn_dir/run_fnc.sh %fnc %s %t && %t | %FileCheck %s --match-full-lines

var a = true;
var b = 0;
if(a){
  b = 10;
} else{
  b = 5;
}
print(b);
// CHECK: 10.000000
