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

var o = {};
o[Symbol.iterator] = function() {
  return {
    next: function() {
      return {value: 0, done: false};
    },
    return: function() {
      // This function should not be called, since quit happens earlier.
      print("Shouldn't call me!");
      throw new TypeError();
    },
  };
};

try {
  var arr = Array.from(o, function(x) {
    print(x);
    // This quit should prevent any JS from executing, including from
    // IteratorClose calling iterator.return.
    quit();
  });
} catch (e) {
  print('Caught:', e);
} finally {
  print('Finally');
}

// QuitError should be uncatchable, and should not be caught by a closing
// iterator in native code.

// CHECK-NEXT: 0
// CHECK-NEXT: Error: Quit
// CHECK-NEXT:     at quit (native)
