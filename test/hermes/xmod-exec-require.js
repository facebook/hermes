/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC -emit-binary -out=%t %s && %hermes -O -b %t | %FileCheck --match-full-lines %s
// RUN: %hermes -O -Xmetro-require=false -target=HBC -emit-binary -out=%t %s && %hermes -O -b %t | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// Tests execution of the Metro require optimization.
// (Test "xmod-requires-opt.js" in the Optimizer directory, which has the same
// code as this one, verifies that the optimization actually happens.)

var modResults = new Array();

function require(modIdx) {
  'noinline'
  var modBefore = modResults[modIdx];
  if (modBefore) return modBefore.exports;
  var exports = {};
  var mod = { exports : exports };

  switch (modIdx) {
  case 0: {
    $SHBuiltin.moduleFactory(
      0,
      function modFact0(global, require, module, exports) {
        function bar() {
          return 17;
        }
        exports.bar = bar;
      })(undefined, require, mod, exports);
    modResults[modIdx] = mod;
    return mod.exports;
  }
  case 1: {
    $SHBuiltin.moduleFactory(
      1,
      function modFact1(global, require, module, exports) {
        // The first one of these should be a cache miss, the
        // second a cache hit.
        var x = require(0).bar() + require(0).bar();
        exports.x = x;
      })(undefined, require, mod, exports);
    modResults[modIdx] = mod;
    return mod.exports;
  }
  default:
  }
}

function outer() {
  'noinline'
  return require(1).x * 2;
}

print(outer());
// CHECK-LABEL: 68

