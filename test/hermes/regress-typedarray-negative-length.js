/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */


// RUN: %hermes %s | %FileCheck %s
// The length argument of new TypedArray(buffer, offset, length) is
// converted with ToIndex (ES2025 23.2.5.1.3 step 5a), so negative and
// non-integer values must throw RangeError instead of clamping to zero.

var buf = new ArrayBuffer(8);

function expectRangeError(length) {
  try {
    new Uint8Array(buf, 0, length);
  } catch (e) {
    print(e instanceof RangeError);
    return;
  }
  throw new Error("no RangeError for length " + String(length));
}

// CHECK: true
// CHECK-NEXT: true
// CHECK-NEXT: true
// CHECK-NEXT: true
expectRangeError(-1);
expectRangeError(-Infinity);
expectRangeError(-1.5);
expectRangeError("-2");

// CHECK-NEXT: 4
print(new Uint8Array(buf, 0, 4).length);
// CHECK-NEXT: 4
print(new Uint8Array(buf, 4, 4).length);
// Undefined length spans the rest of the buffer.
// CHECK-NEXT: 4
print(new Uint8Array(buf, 4).length);
