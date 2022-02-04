/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %node-hermes %s | %FileCheck --match-full-lines %s
// REQUIRES: node-hermes

var {Buffer} = require('buffer');

var buf1 = Buffer.alloc(5);
var buf2 = Buffer.alloc(4098);
try {
  var buf3 = Buffer.alloc(-1); // Does not use ErrorCaptureStackTrace as of now
} catch (e) {
  print('caught:', e.name, e.message);
}
// CHECK: caught: RangeError The argument 'size' is invalid. Received -1

var buf4 = Buffer.alloc(10);
var buf5 = Buffer.alloc(6);
var buf6 = Buffer.alloc(10);
var totalLength = buf4.length + buf5.length + buf6.length;
var buf7 = Buffer.concat([buf4, buf5, buf6]);
print(totalLength);
// CHECK-NEXT: 26

for (let i = 0; i < totalLength; i++) {
  // 97 is the decimal ASCII value for 'a'.
  buf7[i] = 97 + i;
}

print(buf7.toString('utf8')); //Starts after 8 bytes
// CHECK-NEXT: abcdefghijklmnopqrstuvwxyz

var buf8 = Buffer.alloc(10);
for (let i = 0; i < 10; i++) {
  // 97 is the decimal ASCII value for 'a'.
  buf8[i] = 97 + i;
}
print(buf8.toString('utf8', 1, 4));
// CHECK-NEXT: bcde
