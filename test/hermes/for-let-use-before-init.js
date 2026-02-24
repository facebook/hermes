/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-block-scoping %s | %FileCheck --match-full-lines %s
// RUN: (! %hermes -Xenable-tdz -Xes6-block-scoping %s 2>&1 ) | %FileCheck --match-full-lines --check-prefix=CHKTDZ %s

var arr = [];
(function () {
  for(let x=i, i = 0; i < 2; ++i) {
    arr.push(()=>i);
  }
})();

for(f of arr) {
  print(f());
}

// CHECK: 0
// CHECK-NEXT: 1

// CHKTDZ: {{.*}}for-let-use-before-init.js:{{[0-9]+}}:{{[0-9]+}}: error: TDZ violation: reading from uninitialized variable 'i'
