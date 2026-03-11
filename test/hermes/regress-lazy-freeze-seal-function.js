/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

'use strict';

print('freeze');
// CHECK-LABEL: freeze
(function () {
  function frozenFn(a, b) {}

  Object.freeze(frozenFn);

  var frozenDesc = Object.getOwnPropertyDescriptor(frozenFn, 'length');
  print(Object.isFrozen(frozenFn));
  // CHECK-NEXT: true
  print(frozenDesc.configurable, frozenDesc.writable);
  // CHECK-NEXT: false false

  try {
    delete frozenFn.length;
  } catch (e) {
    print(e.name);
    // CHECK-NEXT: TypeError
  }
})();

print('seal');
// CHECK-LABEL: seal
(function () {
  function sealedFn(a, b) {}

  Object.seal(sealedFn);

  var sealedDesc = Object.getOwnPropertyDescriptor(sealedFn, 'length');
  print(Object.isSealed(sealedFn));
  // CHECK-NEXT: true
  print(sealedDesc.configurable, sealedDesc.writable);
  // CHECK-NEXT: false false

  try {
    delete sealedFn.length;
  } catch (e) {
    print(e.name);
    // CHECK-NEXT: TypeError
  }
})();
