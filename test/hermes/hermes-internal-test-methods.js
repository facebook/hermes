/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Xhermes-internal-test-methods %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes -Xhermes-internal-test-methods %t.hbc | %FileCheck --match-full-lines %s
"use strict";

print("HermesInternal");
// CHECK-LABEL: HermesInternal

var desc = Object.getOwnPropertyDescriptor(this, "HermesInternal");
print(desc.enumerable, desc.writable, desc.configurable);
// CHECK-NEXT: false false false

var desc = Object.getOwnPropertyDescriptor(HermesInternal, "detachArrayBuffer");
print(desc.enumerable, desc.writable, desc.configurable);
// CHECK-NEXT: false false false

try { HermesInternal.asdf = 'asdf'; } catch (e) { print('caught', e.name); }
// CHECK-NEXT: caught TypeError
try {
  delete HermesInternal.detachArrayBuffer;
} catch (e) {
  print('caught', e.name);
}
// CHECK-NEXT: caught TypeError
