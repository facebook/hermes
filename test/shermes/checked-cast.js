/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

(function() {

function foo(x): number|string {
    return x;
}

print("Good cast:", foo(10));
//CHECK: Good cast: 10

print("Good cast:", foo("str"));
//CHECK-NEXT: Good cast: str

try {
  foo(true);
} catch (e) {
  print("Bad cast:", e.name, e.message);
}
//CHECK-NEXT: Bad cast: TypeError Checked cast failed

})();

