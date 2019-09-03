// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer


var s = new Set()
var s1 = new Set(new Set([1, 2]));
var s2 = new Set();
s2.add(undefined);
s2.add("abc");
var s3 = new Set([,10,20,,,30,10,,]);

serializeVM(function() {
  print(s);
  //CHECK: [object Set]
  print(s1.has(1), s1.has(2), s1.has(3));
  //CHECK-NEXT: true true false

  // Test some Set functions
  print(s2.size);
  //CHECK-NEXT: 2
  s2.add("cde")
  print(s2.size)
  //CHECK-NEXT: 3
  s2.add("abc")
  print(s2.size)
  //CHECK-NEXT: 3

  print(s3.size);
  //CHECK-NEXT: 4
})
