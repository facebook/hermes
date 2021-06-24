/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %node-hermes %s | %FileCheck --match-full-lines %s

print('Node Requires');
// CHECK-LABEL: Node Requires

try {
  var test1 = require('fs');
} catch (e) {
  print('caught:', e.name, e.message);
}
//CHECK: caught: Error The following module is not supported yet: {{.*}}
