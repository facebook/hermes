/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s 2>&1 | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy -O0 %s 2>&1 | %FileCheck --match-full-lines %s

"use strict";

var obj;
function *gen() {
  obj = {
    get [yield]() { return 1; }
  }
}

var it = gen();
it.next();
it.next('hi');
print(obj.hi);
// CHECK: 1
