// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

var view = new DataView(new ArrayBuffer(8));

var viewWithOffsetAndLength = new DataView(new ArrayBuffer(16), 8, 8);

view.setInt32(4, 5);

var buffer = new ArrayBuffer(16);
var view1 = new DataView(buffer);
HermesInternal.detachArrayBuffer(buffer);

serializeVM(function() {
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
  print(viewWithOffsetAndLength.byteLength);
  // CHECK-NEXT: 8
  print(viewWithOffsetAndLength.byteOffset);
  // CHECK-NEXT: 8

  print(view.getInt32(4));
  // CHECK-NEXT: 5

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
  try {
      view1.setUint32(1,32);
      print('Should\'t reach here');
      // CHECK-NOT: Shouldn't reach here
  } catch (e) {
      print(e.constructor === TypeError);
      // CHECK-NEXT: true
  }
})
