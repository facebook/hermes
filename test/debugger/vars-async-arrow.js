/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheckOrRegen --match-full-lines %s
// REQUIRES: debugger

var x = async () => {
  // Async (generator) debugging isn't supported properly,
  // but it shouldn't crash.
  debugger;
};

x();

// Auto-generated content below. Please do not modify manually.

// CHECK:Break on 'debugger' statement in ?anon_0_x?inner: {{.*}}vars-async-arrow.js[2]:14:3
// CHECK-NEXT:this = function ?anon_0_x?inner() { [bytecode] }
// CHECK-NEXT:Continuing execution
