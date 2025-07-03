/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

print('Error.isError');
// CHECK-LABEL: Error.isError

print(Error.isError(new Error()));
// CHECK-NEXT: true
print(Error.isError(new TypeError()));
// CHECK-NEXT: true
print(Error.isError([]));
// CHECK-NEXT: false
print(Error.isError(new Proxy(new Error(), {})));
// CHECK-NEXT: false
print(Error.isError(123));
// CHECK-NEXT: false
print(Error.isError(null));
// CHECK-NEXT: false
