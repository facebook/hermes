// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: TZ="MST+7" %hermes -O -serialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: TZ="MST+7" %hermes -O -deserialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer
"use strict";

print('Date');
// CHECK-LABEL: Date
print((new Date(2017, 2, 15, 15, 1, 37, 243)).getTime());
// CHECK-NEXT: 1489615297243
print((new Date(2017, 2, 15, 15, 1, 37, 243)).valueOf());
// CHECK-NEXT: 1489615297243
print(Date());
// CHECK-NEXT: {{... ... .. .... ..:..:.. GMT.....}}
// {{... ... [0-9]{2} [0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2} GMT[-+][0-9]{4} }}
print(typeof Date());
// CHECK-NEXT: string
print(new Date(112)[Symbol.toPrimitive]('string'));
// CHECK-NEXT: Wed Dec 31 1969 17:00:00 GMT-0700
var x = new Date('2016-02-15T18:03:57.263-07:00');
print(x.getTime());
// CHECK-NEXT: 1455584637263
var t = 1468631037263;
var x = new Date(t);
print(x.setTime(123456789), x.getTime());
// CHECK-NEXT: 123456789 123456789
