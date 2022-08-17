/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

var global = this;

global.eval('Object.defineProperty(global, "x", {get: function() { return "getx"; }, set: function (v) { print(v); }})');
global.eval('print(x);')
// CHECK: getx
global.eval('var x = 10; print(x);');
// CHECK: 10
// CHECK: getx
global.eval('var x = 20; print(x);');
// CHECK: 20
// CHECK: getx

try {
  global.eval('Object.freeze(global);');
  global.eval('var y = 10;');
} catch (e) {
  print(e);
// CHECK: TypeError: Cannot add new property 'y'
}
