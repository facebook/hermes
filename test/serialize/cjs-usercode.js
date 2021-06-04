/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -commonjs %s %S/cjs-usercode-module-1.js %S/cjs-usercode-module-2.js -serializevm-path=%t
// RUN: %hermes -O -deserialize-file=%t %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

var m = require('./cjs-usercode-module-1.js')

serializeVM(function() {
  print(m.x);
  //CHECK: 42
  print(m.y);
  // CHECK-NEXT: asdf
  print(m.z);
  // CHECK-NEXT: foo
})
