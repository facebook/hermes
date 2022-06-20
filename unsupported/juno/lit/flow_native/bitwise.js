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

function lshift(x, y) {
  return x << y;
}

println('left shift');
// CHECK-LABEL: left shift

println(lshift(1, 3));
// CHECK-NEXT: 8.000000

println(lshift(1.5, 3));
// CHECK-NEXT: 8.000000

println(lshift(0x1, 0x1f));
// CHECK-NEXT: -2147483648.000000

println(lshift(0xdeadbeef, 16));
// CHECK-NEXT: -1091633152.000000

println(lshift(-1, 2));
// CHECK-NEXT: -4.000000

println(lshift(-1, 32));
// CHECK-NEXT: -1.000000

println(lshift(-2147483648, 1));
// CHECK-NEXT: 0.000000


function rshift(x, y) {
  return x >> y;
}

println('right shift');
// CHECK-LABEL: right shift

println(rshift(-14, 2));
// CHECK-NEXT: -4.000000

println(rshift(8, 2));
// CHECK-NEXT: 2.000000

println(rshift(8, 3));
// CHECK-NEXT: 1.000000

println(rshift(18832, 0x82));
// CHECK-NEXT: 4708.000000

println(rshift(0xdeadbeef, 16));
// CHECK-NEXT: -8531.000000

function band(x, y) {
  return x & y;
}

println('bitwise and');
// CHECK-LABEL: bitwise and

println(band(0xdeadbeef, 0xc0ffee));
// CHECK-NEXT: 8437486.000000

function bor(x, y) {
  return x | y;
}

println('bitwise or');
// CHECK-LABEL: bitwise or

println(bor(0xdeadbeef, 0xc0ffee));
// CHECK-NEXT: -554827793.000000

function borzero(x) {
  return x | 0;
}

println('bitwise or with zero');
// CHECK-LABEL: bitwise or with zero

println(borzero(0xdeadbeef));
// CHECK-NEXT: -559038737.000000

println(borzero(1.5));
// CHECK-NEXT: 1.000000

println(borzero(-1000.123));
// CHECK-NEXT: -1000.000000

// Numbers larger than a 32-bit integer should be truncated.
println(borzero(0xffffffff + 1));
// CHECK-NEXT: 0.000000

println(borzero(-0xffffffff - 2));
// CHECK-NEXT: -1.000000

function bxor(x, y) {
  return x ^ y;
}

println('bitwise xor');
// CHECK-LABEL: bitwise xor

println(bxor(0xdeadbeef, 0xc0ffee));
// CHECK-NEXT: -563265279.000000

function bnot(x) {
  return ~x;
}

println('bitwise not');
// CHECK-LABEL: bitwise not

println(bnot(0xdeadbeef));
// CHECK-NEXT: 559038736.000000
