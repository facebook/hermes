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

// This test is just like uncatchable-iterator.js, but it throws the QuitError
// inside the body of return instead, and expects that to override the normal
// JS error thrown from the mapping function.
var o = {};
o[Symbol.iterator] = function() {
  return {
    next: function() {
      return {value: 0, done: false};
    },
    return: function() {
      // This QuitError should take precedence over any other error.
      print('Return called');
      quit();
    },
  };
};

try {
  var arr = Array.from(o, function(x) {
    print(x);
    // Throwing here should cause return to be called, and for the QuitError to
    // be thrown.
    throw new Error();
  });
} catch (e) {
  print('Caught:', e);
} finally {
  print('Finally');
}

// QuitError should be uncatchable, and should not be caught by a closing
// iterator in native code.

// CHECK-NEXT: 0
// CHECK-NEXT: Return called
// CHECK-NEXT: Error: Quit
// CHECK-NEXT:     at quit (native)
