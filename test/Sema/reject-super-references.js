/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes %s -dump-sema -fno-std-globals 2>&1) | %FileCheckOrRegen --match-full-lines %s

var x = super.x;

function foo() {
  return super.x;
}

var obj = {
  m1: function() {
    return super.prop1;
  },
  m2() {
    return function inner() { return super.x };
  },
  m3() {
    return function *inner() { return super.x };
  },
  m4() {
    return async function inner() { return super.x };
  },
  delSup() {
    delete super.x;
  }
};

(function () {
  class A {
    static ["f" + super.f1] = 10;
  };
});

(function () {
  class A { }
  class B extends A {
    static ["f" + super.f1] = 10;
    ["m1" + super.f1]() {}
    static ["m1" + super.f1]() {}
  };
});

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}reject-super-references.js:10:9: error: super not allowed here
// CHECK-NEXT:var x = super.x;
// CHECK-NEXT:        ^~~~~~~
// CHECK-NEXT:{{.*}}reject-super-references.js:13:10: error: super not allowed here
// CHECK-NEXT:  return super.x;
// CHECK-NEXT:         ^~~~~~~
// CHECK-NEXT:{{.*}}reject-super-references.js:18:12: error: super not allowed here
// CHECK-NEXT:    return super.prop1;
// CHECK-NEXT:           ^~~~~~~~~~~
// CHECK-NEXT:{{.*}}reject-super-references.js:21:38: error: super not allowed here
// CHECK-NEXT:    return function inner() { return super.x };
// CHECK-NEXT:                                     ^~~~~~~
// CHECK-NEXT:{{.*}}reject-super-references.js:24:39: error: super not allowed here
// CHECK-NEXT:    return function *inner() { return super.x };
// CHECK-NEXT:                                      ^~~~~~~
// CHECK-NEXT:{{.*}}reject-super-references.js:27:44: error: super not allowed here
// CHECK-NEXT:    return async function inner() { return super.x };
// CHECK-NEXT:                                           ^~~~~~~
// CHECK-NEXT:{{.*}}reject-super-references.js:30:5: error: 'delete' of super property is not allowed
// CHECK-NEXT:    delete super.x;
// CHECK-NEXT:    ^~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}reject-super-references.js:36:19: error: super not allowed here
// CHECK-NEXT:    static ["f" + super.f1] = 10;
// CHECK-NEXT:                  ^~~~~~~~
// CHECK-NEXT:{{.*}}reject-super-references.js:43:19: error: super not allowed here
// CHECK-NEXT:    static ["f" + super.f1] = 10;
// CHECK-NEXT:                  ^~~~~~~~
// CHECK-NEXT:{{.*}}reject-super-references.js:44:13: error: super not allowed here
// CHECK-NEXT:    ["m1" + super.f1]() {}
// CHECK-NEXT:            ^~~~~~~~
// CHECK-NEXT:{{.*}}reject-super-references.js:45:20: error: super not allowed here
// CHECK-NEXT:    static ["m1" + super.f1]() {}
// CHECK-NEXT:                   ^~~~~~~~
// CHECK-NEXT:Emitted 11 errors. exiting.
