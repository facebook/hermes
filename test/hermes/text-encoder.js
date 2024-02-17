/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

print('TextEncoder');
// CHECK-LABEL: TextEncoder

var encoder = new TextEncoder();
print(Object.prototype.toString.call(encoder));
// CHECK-NEXT: [object TextEncoder]

const desc = Object.getOwnPropertyDescriptor(TextEncoder.prototype, 'encoding');
print(desc.enumerable);
// CHECK-NEXT: true
print(desc.configurable);
// CHECK-NEXT: true

print(encoder.encoding);
// CHECK-NEXT: utf-8

try {
  const b = {};
  TextEncoder.prototype.encode.call(b, '');
} catch (e) {
  print(e.message);
  // CHECK-NEXT: TextEncoder.prototype.encode() called on non-TextEncoder object
}

let result = encoder.encode('test');
print(Object.prototype.toString.call(result));
// CHECK-NEXT: [object Uint8Array]

print(result.length, result.join(' '));
// CHECK-NEXT: 4 116 101 115 116

result = encoder.encode('');
print(result.length);
// CHECK-NEXT: 0

result = encoder.encode('\x0D');
print(result.length, result[0]);
// CHECK-NEXT: 1 13

// Test UTF-16 conversion
result = encoder.encode('\u{2191}\u{2192}');
print(Object.prototype.toString.call(result));
// CHECK-NEXT: [object Uint8Array]

print(result.length, result.join(' '));
// CHECK-NEXT: 6 226 134 145 226 134 146

// A surrogate pair
result = encoder.encode('\u{D83D}\u{DE03}');
print(result.length, result.join(' '));
// CHECK-NEXT: 4 240 159 152 131

// Incomplete surrogate
result = encoder.encode('\u{D83D}');
print(result.length, result.join(' '));
// CHECK-NEXT: 3 239 191 189
