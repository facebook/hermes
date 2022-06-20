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

println(10 % 2);
//CHECK: 0.000000

println(10 % 3);
//CHECK-NEXT: 1.000000

println(10 % 0);
//CHECK-NEXT: nan

println(10 % Infinity);
//CHECK-NEXT: 10.000000

println(10 % NaN);
//CHECK-NEXT: nan

println(0 % 10);
//CHECK-NEXT: 0.000000

println(Infinity % 10);
//CHECK-NEXT: nan

println(NaN % 10);
//CHECK-NEXT: nan

println(5.5 % 1.5);
//CHECK-NEXT: 1.000000

println(5.5 % -1.5);
//CHECK-NEXT: 1.000000

println(-5.5 % 1.5);
//CHECK-NEXT: -1.000000

println(-5.5 % -1.5);
//CHECK-NEXT: -1.000000

println(5.5 % 2.5);
//CHECK-NEXT: 0.500000

println(5.5 % -2.5);
//CHECK-NEXT: 0.500000

println(-5.5 % 2.5);
//CHECK-NEXT: -0.500000

println(-5.5 % -2.5);
//CHECK-NEXT: -0.500000
