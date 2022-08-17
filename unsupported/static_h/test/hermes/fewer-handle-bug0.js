/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
var arr = [];
arr.length = 100;
function foo() { return "success"; }
var f = Function.prototype.bind.apply(foo, arr);
print(f());
//CHECK: success
