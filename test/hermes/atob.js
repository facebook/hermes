/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

print('atob');
// CHECK-LABEL: atob
print(atob('YQ=='));
// CHECK-NEXT: a
print(atob('0w=='));
// CHECK-NEXT: Ó
print(atob('000='));
// CHECK-NEXT: ÓM
try {
  atob('\u03A9');
} catch (e) {
  print(e.message);
  // CHECK-NEXT: Not a valid base64 encoded string length
}
print(atob(btoa("a")));
// CHECK-NEXT: a
