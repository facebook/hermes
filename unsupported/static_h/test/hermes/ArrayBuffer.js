/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

/// @name Normal cases
/// @{
var buffer = new ArrayBuffer(16);
// Used for checking the contents of the buffer
var check = new Int8Array(buffer);

print(buffer);
// CHECK: [object ArrayBuffer]

print(buffer.byteLength);
// CHECK-NEXT: 16

// Write some values into it so that the correctness of copying can be checked.
for (var i = 0; i < check.length; i++) {
  check[i] = i;
}

print("Check .isView");
// CHECK-LABEL: Check .isView
print(ArrayBuffer.isView(new Int8Array(buffer)));
// CHECK-NEXT: true
print(ArrayBuffer.isView(new DataView(buffer)));
// CHECK-NEXT: true
print(ArrayBuffer.isView({}));
// CHECK-NEXT: false
print(ArrayBuffer.isView(1));
// CHECK-NEXT: false

print("Check .slice");
// CHECK-LABEL: Check .slice

// copiedBuffer should be an exact copy
var copiedBuffer = buffer.slice(0);
print(buffer.byteLength === copiedBuffer.byteLength);
// CHECK-NEXT: true
var other = new Int8Array(copiedBuffer);
var alltrue = true;
for (var i = 0; i < other.length; i++) {
  if (other[i] !== check[i]) {
    alltrue = false;
  }
}
print("copiedBuffer", alltrue);
// CHECK-NEXT: copiedBuffer true

// This buffer should contain the first 4 bytes
var startBuffer = buffer.slice(0, 4);
print(startBuffer.byteLength);
// CHECK-NEXT: 4
other = new Int8Array(startBuffer);
alltrue = true;
for (var i = 0; i < other.length; i++) {
  if (other[i] !== check[i]) {
    alltrue = false;
  }
}
print("startBuffer", alltrue);
// CHECK-NEXT: startBuffer true

// This buffer should contain the middle 8 bytes from buffer
var middleBuffer = buffer.slice(4, 12);
print(middleBuffer.byteLength);
// CHECK-NEXT: 8
other = new Int8Array(middleBuffer);
alltrue = true;
for (var i = 0; i < other.length; i++) {
  if (other[i] !== check[i + 4]) {
    alltrue = false;
  }
}
print("middleBuffer", alltrue);
// CHECK-NEXT: middleBuffer true

// This buffer should go from 4 from the end to the end
var endBuffer = buffer.slice(-4);
print(endBuffer.byteLength);
// CHECK-NEXT: 4
other = new Int8Array(endBuffer);
alltrue = true;
for (var i = 0; i < other.length; i++) {
  if (other[i] !== check[check.length - 4 + i]) {
    alltrue = false;
  }
}
print("endBuffer", alltrue);
// CHECK-NEXT: endBuffer true

// Check both negative (8 from the end to 4 from the end)
var bothNegativeBuffer = buffer.slice(-8, -4);
print(bothNegativeBuffer.byteLength);
// CHECK-NEXT: 4
other = new Int8Array(bothNegativeBuffer);
alltrue = true;
for (var i = 0; i < other.length; i++) {
  if (other[i] !== check[check.length - 8 + i]) {
    alltrue = false;
  }
}
print("bothNegativeBuffer", alltrue);
// CHECK-NEXT: bothNegativeBuffer true

// @}

/// @name Exceptional cases:
/// @{

/// @name Constructors
/// @{

// Undefined/NaN case
var zeroSizeBuffer = new ArrayBuffer();
print(zeroSizeBuffer.byteLength);
// CHECK-NEXT: 0
// Number larger than 2 ^ 53 == 9.007e15 should fail (ES 2018 draft)
try {
  new ArrayBuffer(1e16);
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: RangeError
// A non-number should truncate to an integral value
var doubleBuffer = new ArrayBuffer(17.4);
print(doubleBuffer.byteLength);
// CHECK-NEXT: 17
/// @}

/// @name Slice
/// @{
// this should be the same as the normal buffer
var tooHighBuffer = buffer.slice(0, 1000);
print(tooHighBuffer.byteLength);
// CHECK-NEXT: 16
var tooLowBuffer = buffer.slice(-1000);
print(tooLowBuffer.byteLength);
// CHECK-NEXT: 16
var overlapBuffer = buffer.slice(6, 4);
print(overlapBuffer.byteLength)
// CHECK-NEXT: 0

/// @}

try { ArrayBuffer.prototype.byteLength; } catch (e) { print('caught', e.name); }
// CHECK-NEXT: caught TypeError
var desc = Object.getOwnPropertyDescriptor(ArrayBuffer.prototype, 'byteLength');
print(desc.enumerable, desc.configurable);
// CHECK-NEXT: false true

/// @}
