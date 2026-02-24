/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s | %FileCheck %s --match-full-lines
// RUN: %hermes -O %s | %FileCheck %s --match-full-lines

print('start');
// CHECK-LABEL:start

function f1(a) {
  return (a?.b?.c)();
}

f1({
  b: {
    c() {
      print('c called');
    },
  },
});
// CHECK-NEXT:c called

try {
  f1({});
} catch (e) {
  print(e.name, e.message);
}
// CHECK-NEXT:TypeError undefined is not a function
