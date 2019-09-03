// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

s = new Set([,10,20,,,30,10,,]);
var it = s.values();

serializeVM(function() {
  var t;
  t = it.next(); print(t.value, t.done);
  //CHECK: undefined false
  t = it.next(); print(t.value, t.done);
  //CHECK-NEXT: 10 false
  t = it.next(); print(t.value, t.done);
  //CHECK-NEXT: 20 false
  t = it.next(); print(t.value, t.done);
  //CHECK-NEXT: 30 false
  t = it.next(); print(t.value, t.done);
  //CHECK-NEXT: undefined true
})
