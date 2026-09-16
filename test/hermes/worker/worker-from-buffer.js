/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec %s | %FileCheck %s --match-full-lines

// The Worker constructor accepts ArrayBuffer, TypedArray, and DataView carrying
// source, faithfully preserving bytes (including bytes >= 0x80).

// Build source bytes from ASCII text, avoiding a dependency on TextEncoder.
function asciiBytes(str) {
  return Uint8Array.from(str, function (c) { return c.charCodeAt(0); });
}

var src = 'onmessage = function() { postMessage("ran"); };';
var bytes = asciiBytes(src);

// 1) ArrayBuffer
var w1 = new Worker(bytes.buffer);
w1.onmessage = function (msg) { print("ab: " + msg); w1.terminate(); };
w1.postMessage("go");

// 2) Uint8Array with non-zero byteOffset (prepend 2 junk bytes, then view).
var padded = new Uint8Array(bytes.length + 2);
padded.set(bytes, 2);
var view = new Uint8Array(padded.buffer, 2, bytes.length);
var w2 = new Worker(view);
w2.onmessage = function (msg) { print("ta: " + msg); w2.terminate(); };
w2.postMessage("go");

// 3) DataView
var w3 = new Worker(new DataView(bytes.buffer));
w3.onmessage = function (msg) { print("dv: " + msg); w3.terminate(); };
w3.postMessage("go");

// 4) Byte fidelity: the source buffer contains a byte >= 0x80 (the two-byte
// UTF-8 encoding of U+00E9 é). The worker echoes its charCode; if the copy
// corrupted high bytes the decoded char would not be 233.
var hi = [];
var prefix = 'onmessage = function() { postMessage("code=" + "';
for (var i = 0; i < prefix.length; i++) hi.push(prefix.charCodeAt(i));
hi.push(0xc3, 0xa9); // UTF-8 for U+00E9 é
var suffix = '".charCodeAt(0)); };';
for (var j = 0; j < suffix.length; j++) hi.push(suffix.charCodeAt(j));
var w4 = new Worker(new Uint8Array(hi).buffer);
w4.onmessage = function (msg) { print("fidelity: " + msg); w4.terminate(); };
w4.postMessage("go");

// Error cases (all synchronous throws at construction).
function expectThrow(fn, label) {
  try { fn(); print(label + ": NO THROW"); }
  catch (e) { print(label + ": " + e.constructor.name); }
}
expectThrow(function () { new Worker(123); }, "number");
expectThrow(function () { new Worker(new ArrayBuffer(0)); }, "empty-ab");
expectThrow(function () { new Worker(new Uint8Array(0)); }, "empty-ta");

// Tamper-resistance: replacing globals must not break native detection.
ArrayBuffer.isView = function () { return false; };
DataView = null;
var w5 = new Worker(bytes.buffer);
w5.onmessage = function (msg) { print("tampered: " + msg); w5.terminate(); };
w5.postMessage("go");

// CHECK: number: TypeError
// CHECK-NEXT: empty-ab: TypeError
// CHECK-NEXT: empty-ta: TypeError
// CHECK-DAG: ab: ran
// CHECK-DAG: ta: ran
// CHECK-DAG: dv: ran
// CHECK-DAG: fidelity: code=233
// CHECK-DAG: tampered: ran
