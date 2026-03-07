/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// Regression test: toLocaleString must use ToLength (not ToUint32) for the
// length property.  ToUint32(2^32) == 0, which incorrectly produces "".
// ToLength(2^32) == 2^32, which must throw RangeError because it exceeds
// the JSArray element limit.

print('toLocaleString ToLength');
// CHECK-LABEL: toLocaleString ToLength

// length = 2^32 should NOT wrap to 0 and return "".
// With the fix it throws RangeError because len > UINT32_MAX.
var obj = {length: 4294967296, 0: 'a'};
try {
  Array.prototype.toLocaleString.call(obj);
  print('no error');
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: RangeError

// length = 2^32 + 2 should also throw RangeError, not wrap to 2.
obj = {length: 4294967298, 0: 'x', 1: 'y'};
try {
  Array.prototype.toLocaleString.call(obj);
  print('no error');
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: RangeError

// length = 2^53 - 1 (max safe integer / max ToLength value) should throw.
obj = {length: 9007199254740991, 0: 'z'};
try {
  Array.prototype.toLocaleString.call(obj);
  print('no error');
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: RangeError
