// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

function className(x) {
    return Object.prototype.toString.call(x);
}

var a;

a = [];

a1 = new Array(3);

a2 = Array(10, 20, 30)

array = new Array(4);
array[0] = 1; array[2] = 2; array[3] = 3;
var proto = {};
array.__proto__ = proto;

Object.defineProperty(proto, 1, {
    get: function get() {
        array.length = 1;
        gc();
        return "foo";
    },
});

a3 = []
a3[0] = 0;
a3[5] = 100;

a4 = [];
a4[4294967294] = 1;

serializeVM(function() {
  print(className(a), a.length);
  //CHECK: [object Array] 0

  print(className(a1), a1.length);
  //CHECK: [object Array] 3

  print(a1[0], a1[1], a1[2], a1[3], a1[4]);
  //CHECK-NEXT: undefined undefined undefined undefined undefined

  for(var i = 0, e = a2.length; i < e; ++i)
      ++a2[i];
  print(a2[0], a2[1], a2[2], a2[3], a2[4]);
  //CHECK-NEXT: 11 21 31 undefined undefined


  print(Array.prototype.concat.call(array));
  // CHECK-NEXT: 1,foo,,


  a3 = Array.prototype.concat.call(a3, a3);
  print(a3.length);
  // CHECK-NEXT: 12
  print(Object.getOwnPropertyDescriptor(a3, 0) === undefined,
        Object.getOwnPropertyDescriptor(a3, 1) === undefined);
  // CHECK-NEXT: false true
  print(Object.getOwnPropertyDescriptor(a3, 5) === undefined,
        Object.getOwnPropertyDescriptor(a3, 6) === undefined);
  // CHECK-NEXT: false false
  print(Object.getOwnPropertyDescriptor(a3, 9) === undefined,
        Object.getOwnPropertyDescriptor(a3, 10) === undefined,
        Object.getOwnPropertyDescriptor(a3, 11) === undefined,
        Object.getOwnPropertyDescriptor(a3, 12) === undefined);
  // CHECK-NEXT: true true false true



  print(a4.length);
  //CHECK-NEXT: 4294967295
  a4[1] = 1;
  print(a4.length);
  //CHECK-NEXT: 4294967295

  a4[4294967295] = 1;
  print(a4.length);
  //CHECK-NEXT: 4294967295

})
