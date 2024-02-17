/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

print('TextEncoder');
// CHECK-LABEL: TextEncoder

var encoder = new TextEncoder();
print(Object.prototype.toString.call(encoder));
// CHECK-NEXT: [object TextEncoder]
