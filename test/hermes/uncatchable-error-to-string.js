/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -O -target=HBC %s 2>&1 ) | %FileCheck %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && (! %hermes %t.hbc 2>&1 ) | %FileCheck %s

print('Start');
// CHECK-LABEL: Start

// Overwrite all type errors to quit upon calling toString.
// Note that it's very important not to overwrite Error.prototype.toString,
// since QuitError's prototype is Error.
TypeError.prototype.toString = function() {
  quit();
};

// QuitError should be uncatchable, and should not be removed by native code.
// In this case, JS.stack is native code.

try {
  throw new TypeError();
} catch (e) {
  // This catch should succeed.
  print('Caught');
  try {
    // This should fail, since the stack will attempt to call toString, which
    // will quit.
    print(e.stack);
  } catch (other) {
    // This catch should not occur, as it is an uncatchable error.
    print('Caught:', other);
  }
}

// CHECK-NEXT: Caught
// CHECK-NEXT: Error: Quit
// CHECK-NEXT:     at quit (native)
