/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s


var C = {x: 2}
C[Symbol.hasInstance] = function(o) {
  // 'this' is C.
  // o is the object being tested for.
  print(this.x, o.isC);
  return o.isC;
}

print({isC:true} instanceof C);
// CHECK: 2 true
// CHECK: true

print({isC:false} instanceof C);
// CHECK: 2 false
// CHECK: false
