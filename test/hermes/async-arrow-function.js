/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

print('async arrow function');
// CHECK-LABEL: async arrow function

// Bind captured state correctly.
(function() {
  function fooOuter(){
    const foo1 = async () => {
      const foo2 = async () => {
        const foo3 = async () => {
          print(this.prop, new.target.prop, arguments[0]);
        }
        foo3();
      }
      foo2();
    }
    this.prop = 'a';
    foo1();
  }
  fooOuter.prop = 'b';
  new fooOuter('c');
})();
// CHECK-NEXT: a b c

// Can't use new.
(function() {
  var asyncArrowFun = async () => { };
  try {
    new asyncArrowFun();
    print("did not throw");
  } catch {
    print("did throw");
  }
})();
// CHECK-NEXT: did throw

// Can use await correctly.
(function() {
  async function asyncOuter() {
    var asyncArrowFun = async () => {
      return await new Promise(resolve => resolve([new.target == undefined, this, ...arguments]));
    };
    return await asyncArrowFun();
  }
  asyncOuter.call(1, 2, 3).then(x => print(x));
})();
// CHECK-NEXT: true,1,2,3

