/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print('callee');
// CHECK-LABEL: callee

function getCallee() {
  return arguments.callee;
}
print(getCallee() === getCallee);
// CHECK-LABEL: true

function checkCallee() {
  print(arguments.callee === checkCallee);
}
checkCallee();
// CHECK-LABEL: true

function leak() {
  return arguments;
}
function leak2() {
  return leak();
}
var a = leak();
print(a.callee === leak);
// CHECK-NEXT: true
var a = leak2();
print(a.callee === leak);
// CHECK-NEXT: true
