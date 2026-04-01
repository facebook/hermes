/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xhermes-internal-test-methods -O %s | %FileCheck %s
// RUN: %shermes -exec %s -Wx,-Xhermes-internal-test-methods | %FileCheck %s

// Tests for TypedArray .buffer, .byteLength, .byteOffset, and .length
// behavior after the underlying ArrayBuffer has been detached.

'use strict';

print("START");
// CHECK-LABEL: START

var constructors = [
  Int8Array,
  Uint8Array,
  Uint8ClampedArray,
  Int16Array,
  Uint16Array,
  Int32Array,
  Uint32Array,
  Float32Array,
  Float64Array,
];

constructors.forEach(function(C) {
  var buf = new ArrayBuffer(64);
  var ta = new C(buf);

  // Before detach, everything should be valid.
  print(C.name, "before detach:",
    ta.buffer === buf,
    ta.byteLength > 0,
    ta.byteOffset === 0,
    ta.length > 0);
  // CHECK-NEXT: {{.*}} before detach: true true true true

  HermesInternal.detachArrayBuffer(buf);

  // After detach, buffer still returns the same (detached) ArrayBuffer.
  print(C.name, "after detach buffer:",
    ta.buffer === buf);
  // CHECK-NEXT: {{.*}} after detach buffer: true

  // After detach, byteLength, byteOffset, and length should all be 0.
  print(C.name, "after detach values:",
    ta.byteLength,
    ta.byteOffset,
    ta.length);
  // CHECK-NEXT: {{.*}} after detach values: 0 0 0
});

// Test with a non-zero byteOffset.
print("non-zero byteOffset after detach");
// CHECK-LABEL: non-zero byteOffset after detach
constructors.forEach(function(C) {
  var elemSize = C.BYTES_PER_ELEMENT;
  // Create buffer large enough to have an offset of at least one element.
  var buf = new ArrayBuffer(elemSize * 4);
  var ta = new C(buf, elemSize, 2);

  print(C.name, "before detach:",
    ta.byteOffset === elemSize,
    ta.byteLength === elemSize * 2,
    ta.length === 2);
  // CHECK-NEXT: {{.*}} before detach: true true true

  HermesInternal.detachArrayBuffer(buf);

  print(C.name, "after detach:",
    ta.byteLength,
    ta.byteOffset,
    ta.length);
  // CHECK-NEXT: {{.*}} after detach: 0 0 0
});

// Test that accessing .buffer on a detached TypedArray does not throw.
print("buffer access does not throw after detach");
// CHECK-LABEL: buffer access does not throw after detach
var buf = new ArrayBuffer(8);
var ta = new Uint8Array(buf);
HermesInternal.detachArrayBuffer(buf);
try {
  var b = ta.buffer;
  print("no throw, buffer is ArrayBuffer:", b instanceof ArrayBuffer);
  // CHECK-NEXT: no throw, buffer is ArrayBuffer: true
  print("detached buffer byteLength:", b.byteLength);
  // CHECK-NEXT: detached buffer byteLength: 0
} catch (e) {
  print("threw:", e.constructor.name);
}

// Test that detaching does not affect other TypedArray views sharing the
// same buffer -- they should all reflect the detached state.
print("multiple views on same detached buffer");
// CHECK-LABEL: multiple views on same detached buffer
var shared = new ArrayBuffer(64);
var u8 = new Uint8Array(shared);
var i32 = new Int32Array(shared);
var f64 = new Float64Array(shared);
HermesInternal.detachArrayBuffer(shared);
print("Uint8Array:", u8.byteLength, u8.byteOffset, u8.length);
// CHECK-NEXT: Uint8Array: 0 0 0
print("Int32Array:", i32.byteLength, i32.byteOffset, i32.length);
// CHECK-NEXT: Int32Array: 0 0 0
print("Float64Array:", f64.byteLength, f64.byteOffset, f64.length);
// CHECK-NEXT: Float64Array: 0 0 0

// Test that buffer references are the same detached object.
print("buffer identity after detach");
// CHECK-LABEL: buffer identity after detach
print(u8.buffer === shared, i32.buffer === shared, f64.buffer === shared);
// CHECK-NEXT: true true true

// Test zero-length TypedArray on detached buffer.
print("zero-length typed array after detach");
// CHECK-LABEL: zero-length typed array after detach
var buf2 = new ArrayBuffer(0);
var empty = new Uint8Array(buf2);
print("before:", empty.buffer === buf2, empty.byteLength, empty.length);
// CHECK-NEXT: before: true 0 0
HermesInternal.detachArrayBuffer(buf2);
print("after:", empty.buffer === buf2, empty.byteLength, empty.length);
// CHECK-NEXT: after: true 0 0

print("DONE");
// CHECK-LABEL: DONE
