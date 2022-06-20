/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fn_dir/run_fnc.sh %fnc %s %t && %t | %FileCheck %s --match-full-lines

function println(x){
  print(x);
  print("\n");
}

println(true && false);
// CHECK-LABEL: false
println(false || true);
// CHECK-NEXT: true
println(!false);
// CHECK-NEXT: true
println(null ?? 'foo');
// CHECK-NEXT: foo

function doNotCall(){
  throw "foo";
}

println(false && doNotCall());
// CHECK-LABEL: false
println(true || doNotCall());
// CHECK-NEXT: true
println(5 ?? doNotCall());
// CHECK-NEXT: 5.000000
