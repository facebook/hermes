/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print('logical assignment');
// CHECK-LABEL: logical assignment

var x = true;
print(x &&= false);
// CHECK-NEXT: false
print(x);
// CHECK-NEXT: false

var x = true;
print(x &&= true);
// CHECK-NEXT: true
print(x);
// CHECK-NEXT: true

var x = false;
print(x ||= true);
// CHECK-NEXT: true
print(x);
// CHECK-NEXT: true

var x = false;
print(x ??= true);
// CHECK-NEXT: false
print(x);
// CHECK-NEXT: false

var x = null;
print(x ??= 3);
// CHECK-NEXT: 3
print(x);
// CHECK-NEXT: 3

var x = undefined;
print(x ??= 3);
// CHECK-NEXT: 3
print(x);
// CHECK-NEXT: 3

function testComplex(x, y, z) {
  print(x ||= y || z);
  print(x);
}
testComplex(false, false, true);
// CHECK-NEXT: true
// CHECK-NEXT: true
testComplex(false, false, false);
// CHECK-NEXT: false
// CHECK-NEXT: false
testComplex(true, false, false);
// CHECK-NEXT: true
// CHECK-NEXT: true

var obj = {
  get x() {
    print('getter x');
    return null;
  },

  set x(val) {
    print('setter x:', val);
  },
};

obj.x &&= 3;
// CHECK-NEXT: getter x
// CHECK-NOT: setter x: {{.*}}
obj.x ||= 3;
// CHECK-NEXT: getter x
// CHECK-NEXT: setter x: 3
obj.x ??= 3;
// CHECK-NEXT: getter x
// CHECK-NEXT: setter x: 3
