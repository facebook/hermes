/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

var a = {}
Object.defineProperty(a, 'mypropname', { writable: false });
try {
  a.mypropname = 42;
} catch(e) {
  print('caught', e.name, e.message);
}
// CHECK: caught TypeError Cannot assign to read-only property 'mypropname'
