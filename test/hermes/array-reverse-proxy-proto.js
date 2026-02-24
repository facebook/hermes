/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

function foo(){
  let par = new Proxy({1: 2, 2: 3}, {});
  let arr = [1,,,4];
  arr.__proto__.__proto__ = par;
  arr.reverse();
  print(JSON.stringify(arr));
}
foo();

//CHECK: [4,3,2,1]
