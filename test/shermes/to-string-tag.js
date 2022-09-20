/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

var obj = {};

obj[Symbol.toStringTag] = "MyFavoriteObject";
print(obj.toString());
// CHECK: [object MyFavoriteObject]

// Not a string, use "Object" instead.
obj[Symbol.toStringTag] = 123;
print(obj.toString());
// CHECK: [object Object]

// Ensure the override works for built-ins as well.
Boolean.prototype[Symbol.toStringTag] = 'asdf';
print(Object.prototype.toString.call(true));
// CHECK: [object asdf]

print((new Int8Array(10))[Symbol.toStringTag]);
// CHECK: Int8Array
