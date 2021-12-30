/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

print('escape');
// CHECK-LABEL: escape
print(escape('foo'));
// CHECK-NEXT: foo
print(escape('asdf ASDF'));
// CHECK-NEXT: asdf%20ASDF
print(escape('!@#$%^&*()'));
// CHECK-NEXT: %21@%23%24%25%5E%26*%28%29
print(escape('આકાશ'));
// CHECK-NEXT: %u0A86%u0A95%u0ABE%u0AB6
print(escape('integral ∮'));
// CHECK-NEXT: integral%20%u222E

print('unescape');
// CHECK-LABEL: unescape
print(unescape('foo'));
// CHECK-NEXT: foo
print(unescape('asdf%20ASDFASDF'));
// CHECK-NEXT: asdf ASDFASDF
print(unescape('%21@%23%24%25%5E%26*%28%29'));
// CHECK-NEXT: !@#$%^&*()
print(unescape('%u0A86%u0a95%u0AbE%u0AB6'));
// CHECK-NEXT: આકાશ
print(unescape('integral%20%u222e'));
// CHECK-NEXT: integral ∮
print(unescape('%#!'));
// CHECK-NEXT: %#!
print(unescape('asd%%'));
// CHECK-NEXT: asd%%
print(unescape('asd%u123'));
// CHECK-NEXT: asd%u123
print(unescape('asd%u123x'));
// CHECK-NEXT: asd%u123x
