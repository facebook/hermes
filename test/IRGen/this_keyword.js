/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

//CHECK: function f1()
//CHECK: frame = []
//CHECK:   %BB0:
//CHECK:     %0 = ReturnInst %this
function f1() {
  return this;
}

//CHECK: function f2()
//CHECK: frame = []
//CHECK:   %BB0:
//CHECK:     %0 = ReturnInst %this
function f2(){
  "use strict"; // see strict mode
  return this;
}

