/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

function getGetter() {
  "use strict";
  var desc = Object.getOwnPropertyDescriptor(arguments, 'callee');
  return desc.get;
}

print('ThrowTypeError');
// CHECK-LABEL: ThrowTypeError
var getter = getGetter();
print(getter.length);
// CHECK-NEXT: 0
var desc = Object.getOwnPropertyDescriptor(getter, 'length');
print(desc.writable);
// CHECK-NEXT: false
print(desc.enumerable);
// CHECK-NEXT: false
print(desc.configurable);
// CHECK-NEXT: false
