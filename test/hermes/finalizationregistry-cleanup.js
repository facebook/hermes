/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -Xmicrotask-queue -O -gc-sanitize-handles=1 -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec -Wx,-Xmicrotask-queue -Wx,-gc-sanitize-handles=1 %s | %FileCheck --match-full-lines %s

'use strict';

print('FinalizationRegistry');
// CHECK-LABEL: FinalizationRegistry

// A helper function to ensure that the created temporary object becomes dead
// after this function returns (since the stack frame gets popped out), so a
// following gc() can successfully collect it.
function register(fr, token) {
  fr.register({}, token);
}

let called = false;
// Resigter/unregister cells in cleanup, this should still work.
var fr = new FinalizationRegistry(heldValue => {
  if (!called) {
    called = true;
    // The callback for this registered cell will be triggered in second
    // checkpoint.
    register(fr, 'heldValue1 Inner');
    gc();
  }
  print('cleanup', heldValue);
});
register(fr, 'heldValue1');

var fr2 = new FinalizationRegistry(heldValue => {
  fr2.unregister({});
  print('cleanup', heldValue);
});
register(fr2, 'heldValue2');

print('Done');
// CHECK-NEXT: Done
gc();
// Add a timeout so that the microtask checkpoint will be performed two times.
setTimeout(() => {}, 0);
// CHECK-NEXT: cleanup heldValue1
// CHECK-NEXT: cleanup heldValue2
// CHECK-NEXT: cleanup heldValue1 Inner
