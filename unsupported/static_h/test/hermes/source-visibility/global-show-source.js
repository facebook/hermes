/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s
// UNSUPPORTED: serializer

'show source';

function foo(x) { return x }
function bar(y) { return y }
function baz(z) { return z }

// HideSource can override ShowSource
function hideSource (x) {
    'hide source';
    return x;
}

print("global show source");
// CHECK-LABEL: global show source

print(foo.toString());
// CHECK-NEXT: function foo(x) { return x }

print(bar.toString());
// CHECK-NEXT: function bar(y) { return y }

print(baz.toString());
// CHECK-NEXT: function baz(z) { return z }

print(hideSource.toString());
// CHECK-NEXT: function hideSource() { [native code] }
