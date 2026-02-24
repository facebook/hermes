/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %hermes - | %FileCheck --match-full-lines %s
// REQUIRES: !debug_options

// Create a function that exhausts the number of property cache entries.
// Ensure that various properties in the object are still correctly accessed.

print("var o = {a0: 4, a100: 72, a512: 10, a675: 23, a999: 200};");
print("(function foo() { let val, sum = 0;");

for(let i = 0; i < 1024; ++i){
  print("val  = o.a" + i + ";");
  print("sum += val ? val : 0;");
}

print("print('Sum:', sum);")

print("})();");

// CHECK: Sum: 309
