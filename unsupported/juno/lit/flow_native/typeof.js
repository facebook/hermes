/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fn_dir/run_fnc.sh %fnc %s %t && %t | %FileCheck %s --match-full-lines

var x = [5, undefined, null, "foo", {}, function(){}]

for(var i = 0; i < 6; i++){
  print(typeof x[i]);
  print("\n");
}

// CHECK: number
// CHECK-NEXT: undefined
// CHECK-NEXT: object
// CHECK-NEXT: string
// CHECK-NEXT: object
// CHECK-NEXT: function
