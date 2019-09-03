// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

/// @name Normal cases
/// @{

var buffer = new ArrayBuffer(4);
// Used for checking the contents of the buffer
var check = new Int8Array(buffer);

// Write some values into it
for (var i = 0; i < check.length; i++) {
  check[i] = i;
}

serializeVM(function () {
  print(buffer);
  // CHECK: [object ArrayBuffer]

  print(buffer.byteLength);
  // CHECK-NEXT: 4

  for (var i = 0; i < check.length; i++) {
    print(check[i]);
  }
  // CHECK-NEXT:0
  // CHECK-NEXT:1
  // CHECK-NEXT:2
  // CHECK-NEXT:3

  // See if we can still add to it.
  for (var i = 0; i < check.length; i++) {
    check[i] += 1;
  }

  for (var i = 0; i < check.length; i++) {
    print(check[i]);
  }
  // CHECK-NEXT:1
  // CHECK-NEXT:2
  // CHECK-NEXT:3
  // CHECK-NEXT:4

  try { ArrayBuffer.prototype.byteLength; } catch (e) { print('caught', e.name); }
  // CHECK-NEXT: caught TypeError
  var desc = Object.getOwnPropertyDescriptor(ArrayBuffer.prototype, 'byteLength');
  print(desc.enumerable, desc.configurable);
  // CHECK-NEXT: false true

  /// @}
})
