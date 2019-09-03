// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

var date = new Date(2017, 2, 15, 15, 1, 37, 243);
var date2 = new Date(69, 6, 20, 20, 18)
var date3 = new Date(84, 5)
var date4 = new Date(446947200000)
var date5 = new Date(0)
var date6 = new Date(1e81)
var date7 = new Date()

var t = 1468631037263;
var x = new Date(t);

serializeVM(function() {
  print('Date');
  // CHECK-LABEL: Date
  print(date.getTime());
  // CHECK-NEXT: 1489615297243
  print(date.valueOf());
  // CHECK-NEXT: 1489615297243
  print(date2.getTime());
  // CHECK-NEXT: -14157720000
  print(date3.getTime());
  // CHECK-NEXT: 454921200000
  print(date4.getTime());
  // CHECK-NEXT: 446947200000
  print(date5.getTime());
  // CHECK-NEXT: 0
  print(date6.getTime());
  // CHECK-NEXT: NaN
  print(date7);
  // CHECK-NEXT: {{... ... .. .... ..:..:.. GMT.....}}
  // {{... ... [0-9]{2} [0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2} GMT[-+][0-9]{4} }}

  print(x.setTime(123456789), x.getTime());
  // CHECK-NEXT: 123456789 123456789
})
