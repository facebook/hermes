/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

'use strict';
var p = new Proxy({}, {});
try {
  globalThis.__proto__ = p;
} catch (e) {
  print(e.name, e.message);
}
result = 10;

print('done');
// CHECK: done
