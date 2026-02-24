/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

var proto = {
  m() {
    return ' proto m';
  }
};
var object = {
  get ['a']() { return 'a' + super.m(); },
};
Object.setPrototypeOf(object, proto);
print(object.a);
// CHECK: a proto m

// Test that accessors without computed names can also refer to super.
(function () {
  let v1 = {
    get a() {
      let x = super.m;
      print(x);
    }
  }
  let parent = { m: 12 };
  v1.a;
// CHECK-NEXT: undefined
  Object.setPrototypeOf(v1, parent);
  v1.a;
// CHECK-NEXT: 12
})();
