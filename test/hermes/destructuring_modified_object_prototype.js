/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Test that the excluded items object produced for destructuring does not use
// Object.prototype.
(function(){
  var a = {a: 0, b: 0};
  Object.prototype.b = 0;
  var {a: x, ...y} = a;
  print(JSON.stringify(y));
})();

// CHECK-LABEL: {"b":0}
