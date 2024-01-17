/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

print('btoa');
// CHECK-LABEL: btoa
print(btoa('123'));
// CHECK-NEXT: MTIz
try {
  btoa('\u03A9');
} catch (e) {
  print(e.message);
  // CHECK-NEXT: Found invalid character when converting to base64
}
