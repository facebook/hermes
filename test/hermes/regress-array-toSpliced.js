/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

print("toSpliced");
// CHECK: toSpliced 

(function () {
  var arrayLike = {
    "9007199254740989": 42,
    "9007199254740990": 43,
    "9007199254740991": 44, // 2^53 - 1 = 9007199254740991 is the max length
    "9007199254740992": 45,
    "9007199254740994": 46,
    length: 9007199254740991 + 10
  };
  let skip = 2 ** 53 - 3;
  var result = Array.prototype.toSpliced.call(arrayLike, 0, skip);
  print(result.length);
// CHECK-NEXT: 2
  print(result[0], result[1]);
// CHECK-NEXT: 42 43
})();

(function () {
  var arrayLike = {
    "4294967295": 10,
    "4294967296": 11,
    "4294967297": 12,
    "4294967298": 13,
    length: 4294967298 
  };
  let skip = 4294967295;
  var result = Array.prototype.toSpliced.call(arrayLike, 0, skip);
  print(result.length);
// CHECK-NEXT: 3
  print(result[0], result[1], result[2]);
// CHECK-NEXT: 10 11 12
})();
