/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// REQUIRES: core_extensions
// RUN: LC_ALL=en_US.UTF-8 %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s

// Regression test: TextEncoder.encodeInto() and TextDecoder.decode() accept a
// duck-typed TypedArray view -- any object exposing buffer/byteOffset/byteLength.
// The view geometry was previously trusted without checking that
// byteOffset + byteLength stays within the backing ArrayBuffer, so a plain
// object with out-of-bounds values produced an out-of-bounds write/read relative
// to the buffer. Such views must now throw instead. Both APIs share the same
// bounds check, so exercising the write path is sufficient to cover it.
"use strict";

print('typedarray-view-bounds');
// CHECK-LABEL: typedarray-view-bounds

const enc = new TextEncoder();

// Out of bounds because byteOffset is past the end (exploit shape).
try {
  const ab = new ArrayBuffer(16);
  enc.encodeInto("A".repeat(0x1000), {
    buffer: ab,
    byteOffset: 0x100000,
    byteLength: 0x1000,
  });
  print("FAIL: OOB byteOffset did not throw");
} catch (e) {
  print(e.message);
  // CHECK-NEXT: The second argument should be a Uint8Array
}

// Out of bounds because byteLength runs past the end (offset is valid).
try {
  const ab = new ArrayBuffer(16);
  enc.encodeInto("A".repeat(17), {buffer: ab, byteOffset: 0, byteLength: 17});
  print("FAIL: OOB byteLength did not throw");
} catch (e) {
  print(e.message);
  // CHECK-NEXT: The second argument should be a Uint8Array
}

// In bounds: byteOffset + byteLength == size must be accepted (off-by-one guard).
const ab1 = new ArrayBuffer(10);
let r = enc.encodeInto("ABCD", {buffer: ab1, byteOffset: 6, byteLength: 4});
print(r.read + " " + r.written + " " + new Uint8Array(ab1).slice(6).join(","));
// CHECK-NEXT: 4 4 65,66,67,68

// In bounds: empty view at the very end (byteOffset == size, byteLength == 0).
const ab2 = new ArrayBuffer(4);
r = enc.encodeInto("xyz", {buffer: ab2, byteOffset: 4, byteLength: 0});
print(r.read + " " + r.written);
// CHECK-NEXT: 0 0

// A real Uint8Array still round-trips through the normal path.
const view = new Uint8Array(8);
r = enc.encodeInto("hello", view);
print(r.read + " " + r.written + " " + view.slice(0, 5).join(","));
// CHECK-NEXT: 5 5 104,101,108,108,111

print('done');
// CHECK-NEXT: done
