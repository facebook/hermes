/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s
// Test that string conversion preserves the semantics of addition.

print("add-empty-string");
//CHECK-LABEL: add-empty-string

var o = Object('x');
o.valueOf = function() { return 'y' };
print(o + '');
//CHECK-NEXT: y
print('' + o);
//CHECK-NEXT: y

String.prototype.valueOf = function() { return 'y' };
var o = Object('x');
print(o + '');
//CHECK-NEXT: y
print('' + o);
//CHECK-NEXT: y
