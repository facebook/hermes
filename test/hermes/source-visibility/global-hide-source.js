/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s
// UNSUPPORTED: serializer

'hide source';

function foo(x) { return x }
function bar(y) { return y }
function baz(z) { return z }

// ShowSource cannot override HideSource.
function hideSource (x) {
    'show source';
    return x;
}

print("global hide source")
// CHECK-LABEL: global hide source

print(foo.toString());
// CHECK-NEXT: function foo() { [native code] }

print(bar.toString());
// CHECK-NEXT: function bar() { [native code] }

print(baz.toString());
// CHECK-NEXT: function baz() { [native code] }

print(hideSource.toString());
// CHECK-NEXT: function hideSource() { [native code] }
