/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fcache-new-object %s | %FileCheck --match-full-lines %s

function foo(){
  this.x = 1;
  this.y = 2;
  this.z = 3;
}

foo.prototype = { set y(v){ print("setter invoked"); } };
var obj = new foo();
// CHECK-LABEL: setter invoked
print(JSON.stringify(obj));
// CHECK-NEXT: {"x":1,"z":3}
