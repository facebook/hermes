/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
"use strict";

function printCallee() {
  print(arguments.callee);
}
function printCaller() {
  print(arguments.caller);
}
function leak() {
  return arguments;
}

print('callee/caller');
// CHECK-LABEL: callee/caller
try { printCallee() } catch(e) { print(e.name) }
// CHECK-NEXT: TypeError
try { printCaller() } catch(e) { print(e.name) }
// CHECK-NEXT: TypeError
try { print(leak().callee); } catch(e) { print(e.name) }
// CHECK-NEXT: TypeError
