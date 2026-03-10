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

let fr1 = new FinalizationRegistry(heldValue => {
  // This would free the second fr, so its callback should never be called.
  gc();
  print('cleanup', heldValue);
});
let fr2 = new FinalizationRegistry(() => {
  print('Error');
});
register(fr1, 'heldValue1');
register(fr2, 'heldValue2');

gc();
// CHECK-NEXT: cleanup heldValue1
// CHECK-NOT: Error

setTimeout(() => {
  // Reference fr1 here to ensure that it's still alive when the shermes event
  // loop is running.
  fr1.unregister({});
}, 0);
