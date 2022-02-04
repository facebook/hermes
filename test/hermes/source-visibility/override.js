/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s
// UNSUPPORTED: serializer

// Inner HideSource can override outer ShowSource
function showSource1() {
    'show source';
    function hideSource() {
        'hide source';
    }
    return hideSource;
}

// Inner ShowSource cannot override outer HideSource/Sensitive
function sensitive() {
    'sensitive';
    function showSource2() {
        'show source';
    }
    return showSource2;
}

print("complicated overriding");
// CHECK-LABEL: complicated overriding

print(showSource1.toString());
// CHECK-NEXT: function showSource1() {
// CHECK-NEXT:     'show source';
// CHECK-NEXT:     function hideSource() {
// CHECK-NEXT:         'hide source';
// CHECK-NEXT:     }
// CHECK-NEXT:     return hideSource;
// CHECK-NEXT: }

print(sensitive.toString());
// CHECK-NEXT: function sensitive() { [native code] }

// Invoking these functions after their toString to test out lazy.
var hideSource = showSource1();
var showSource2 = sensitive();

print(hideSource.toString());
// CHECK-NEXT: function hideSource() { [native code] }

print(showSource2.toString());
// CHECK-NEXT: function showSource2() { [native code] }
