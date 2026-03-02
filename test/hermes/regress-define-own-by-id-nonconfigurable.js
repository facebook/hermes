/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

print("DefineOwnById configurable/enumerable checks");
// CHECK-LABEL: DefineOwnById configurable/enumerable checks

// Test 1: DefineOwnById must throw when redefining a non-configurable property.
// Class field `field = 2` emits DefineOwnById with default flags
// (writable, enumerable, configurable all true). If 'field' already exists
// as non-configurable, the spec requires a TypeError because configurable
// cannot be changed from false to true.
(function() {
  class Base {
    constructor() {
      Object.defineProperty(this, 'field', {
        value: 1,
        writable: true,
        enumerable: true,
        configurable: false,
      });
    }
  }
  class Child extends Base {
    field = 2;
  }
  try {
    new Child();
    print("no error");
  } catch (e) {
    print("caught", e.constructor.name);
    // CHECK-NEXT: caught TypeError
  }
})();

// Test 2: DefineOwnById must update enumerable from false to true.
// The class field definition uses default flags (enumerable: true).
// If the existing property is configurable but non-enumerable, the slow path
// should correctly update enumerable to true.
(function() {
  class Base {
    constructor() {
      Object.defineProperty(this, 'field', {
        value: 1,
        writable: true,
        enumerable: false,
        configurable: true,
      });
    }
  }
  // Sanity check: Base instance is non-enumerable.
  var base = new Base();
  var baseDesc = Object.getOwnPropertyDescriptor(base, 'field');
  print("value:", baseDesc.value, "enumerable:", baseDesc.enumerable);
  // CHECK-NEXT: value: 1 enumerable: false

  class Child extends Base {
    field = 2;
  }
  var obj = new Child();
  var desc = Object.getOwnPropertyDescriptor(obj, 'field');
  print("value:", desc.value, "enumerable:", desc.enumerable);
  // CHECK-NEXT: value: 2 enumerable: true
})();
