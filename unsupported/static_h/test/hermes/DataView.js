/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xhermes-internal-test-methods -O %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -emit-binary -out %t.hbc %s && %hermes -Xhermes-internal-test-methods %t.hbc | %FileCheck --match-full-lines %s

print("Check .length");
// CHECK-LABEL: Check .length
print(DataView.length);
// CHECK-NEXT: 1

var view = new DataView(new ArrayBuffer(8));

print("Check .buffer");
// CHECK-LABEL: Check .buffer
print(view.buffer);
// CHECK-NEXT: [object ArrayBuffer]
print(view.byteLength);
// CHECK-NEXT: 8
try { DataView.prototype.buffer; } catch (e) { print('caught', e.name); }
// CHECK-NEXT: caught TypeError

print("Check constructor with byteOffset and byteLength");
// CHECK-LABEL: Check constructor with byteOffset and byteLength
var viewWithOffsetAndLength = new DataView(new ArrayBuffer(16), 8, 8);
print(viewWithOffsetAndLength.byteLength);
// CHECK-NEXT: 8
print(viewWithOffsetAndLength.byteOffset);
// CHECK-NEXT: 8

print("Check get and set at front (BE)");
// CHECK-LABEL: Check get and set at front (BE)
print(view.getInt8(0));
// CHECK-NEXT: 0
view.setInt8(0, 4);
print(view.getInt8(0));
// CHECK-NEXT: 4
// undo
view.setInt8(0, 0);

print("Check get and set in middle (BE)");
// CHECK-LABEL: Check get and set in middle (BE)
print(view.getInt8(4));
// CHECK-NEXT: 0
view.setInt8(4, 5);
print(view.getInt8(4));
// CHECK-NEXT: 5
// undo
view.setInt8(4, 0);

print("Check get and set with multibyte value (BE)");
// CHECK-LABEL: Check get and set with multibyte value (BE)
view.setInt32(0, 5);
print(view.getInt32(0));
// CHECK-NEXT: 5
// undo
view.setInt32(0, 0);

print("Check get and set in middle with multibyte value (BE)");
// CHECK-LABEL: Check get and set in middle with multibyte value (BE)
print(view.getInt32(4));
// CHECK-NEXT: 0
view.setInt32(4, 5);
print(view.getInt32(4));
// CHECK-NEXT: 5
// undo
view.setInt32(4, 0);

print("Check get and set with big multibyte value (BE)");
// CHECK-LABEL: Check get and set with big multibyte value (BE)
view.setInt32(0, 1 << 30);
print(view.getInt32(0));
// CHECK-NEXT: 1073741824
// undo
view.setInt32(0, 0);

print("Check difference between LE and BE");
// CHECK-LABEL: Check difference between LE and BE
view.setInt32(0, 1 << 30, false);
print(view.getInt32(0, false));
// CHECK-NEXT: 1073741824
print(view.getInt32(0, true));
// CHECK-NEXT: 64
// now in reverse
view.setInt32(0, 1 << 30, true);
print(view.getInt32(0, false));
// CHECK-NEXT: 64
print(view.getInt32(0, true));
// CHECK-NEXT: 1073741824
view.setInt32(0, 0);

print("Check reading same value as different signs");
// CHECK-LABEL: Check reading same value as different signs
view.setInt16(0, -1);
print(view.getInt16(0));
// CHECK-NEXT: -1
print(view.getUint16(0));
// CHECK-NEXT: 65535
view.setInt16(0, 0);

print("Check float sanitization");
// CHECK-LABEL: Check float sanitization
view.setUint32(0, 0xffffffff);
print(view.getFloat32(0));
// CHECK-NEXT: NaN
view.setUint32(0, 0);

print("Check conversion");
// CHECK-LABEL: Check conversion
view.setInt16(0, 4160782224, true);
print(view.getInt16(0));
// CHECK-NEXT: -28545

print("Check descriptors");
// CHECK-LABEL: Check descriptors
var desc = Object.getOwnPropertyDescriptor(DataView.prototype, 'buffer');
print(desc.get.name, desc.enumerable, desc.configurable);
// CHECK-NEXT: get buffer false true
var desc = Object.getOwnPropertyDescriptor(DataView.prototype, 'byteLength');
print(desc.get.name, desc.enumerable, desc.configurable);
// CHECK-NEXT: get byteLength false true
var desc = Object.getOwnPropertyDescriptor(DataView.prototype, 'byteOffset');
print(desc.get.name, desc.enumerable, desc.configurable);
// CHECK-NEXT: get byteOffset false true

print("Check get and set on detached ArrayBuffer");
// CHECK-LABEL: Check get and set on detached ArrayBuffer
var buffer = new ArrayBuffer(16);
var view = new DataView(buffer);
HermesInternal.detachArrayBuffer(buffer);
try {
    view.setUint32(1,32);
    print('Should\'t reach here');
    // CHECK-NOT: Shouldn't reach here
} catch (e) {
    print(e.constructor === TypeError);
    // CHECK-NEXT: true
}
