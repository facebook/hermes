/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -w -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -w -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

var obj = {a: 0, b: 1};
for (var p in obj) {
  print(p, obj[p]);
}
//CHECK: a 0
//CHECK-NEXT: b 1

print('getter/setter redefinition');
//CHECK-LABEL: getter/setter redefinition
obj = {
  set a(v) { print("set"); },
  'b': 'b',
  'a': 'c',
  get a() { return 'a'; },
};

for (var p in obj) {
  print(p, obj[p]);
}
//CHECK-NEXT: a a
//CHECK-NEXT: b b
obj.a = 0;
print(obj.a);
//CHECK-NEXT: a

obj = {
  get a(){ print("get"); },
  a : print("side effect"),
  a : 6,
}
//CHECK-NEXT: side effect

print(obj.a)
//CHECK-NEXT: 6

obj = {get 10 (){ return 42 }, 10 : 1, 11: 3}
print(obj[10])
//CHECK-NEXT: 1

print('value redefinition')
//CHECK-LABEL: value redefinition

function make(n){
  print("Making ", n);
  return n;
}

obj = {
  a: make(1),
  b: make(2),
  a: make(3),
};
//CHECK-NEXT: Making 1
//CHECK-NEXT: Making 2
//CHECK-NEXT: Making 3

for (var p in obj) {
  print(p, obj[p]);
}
//CHECK-NEXT: a 3
//CHECK-NEXT: b 2
