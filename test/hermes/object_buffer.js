/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// This object is created from object buffer. Check it works.
var obj = {
  a : undefined,
  b : 1,
  c : {a : 3, b : 2},
  f : 1,
  g : 1,
  h : 1,
  i : 1,
  j : 1,
  k : 1,
  l : 1,
  m : 1,
  n : 1,
  o : function(x) {return x},
  p : 1,
  q : 1,
};

//CHECK: undefined
print(obj.a);
//CHECK-NEXT: 1
print(obj.f);
//CHECK-NEXT: 8
print(obj.o(8));
//CHECK-NEXT: 3
print(obj.c.a);
//CHECK-NEXT: 2
print(obj.c.b);

var obj2 = {
  a: 1,
  get b() {
    return 3;
  },
  c: 2,
  set b(x) {
    return;
  },
  d: 5,
  e: 6
};
print(Object.keys(obj2));
//CHECK-NEXT: a,b,c,d,e
