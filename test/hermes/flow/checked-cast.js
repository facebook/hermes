/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -typed -O0 %s | %FileCheck --match-full-lines %s

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

try {
  foo({});
} catch (e) {
  print("Bad cast:", e.name, e.message);
}
//CHECK-NEXT: Bad cast: TypeError Checked cast failed

try {
  foo(() => {});
} catch (e) {
  print("Bad cast:", e.name, e.message);
}
//CHECK-NEXT: Bad cast: TypeError Checked cast failed

// Test nullable type (number | null | void).
function bar(x): ?number {
    return x;
}

print("Good cast:", bar(42));
//CHECK-NEXT: Good cast: 42

print("Good cast:", bar(null));
//CHECK-NEXT: Good cast: null

print("Good cast:", bar(undefined));
//CHECK-NEXT: Good cast: undefined

try {
  bar("str");
} catch (e) {
  print("Bad cast:", e.name, e.message);
}
//CHECK-NEXT: Bad cast: TypeError Checked cast failed

})();
