/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -Xparse-component-syntax -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

type Foo = component Foo();

// CHECK: {{.*}}:10:22: error: component type annotations should not contain a name
// CHECK-NEXT: type Foo = component Foo();
// CHECK-NEXT:                      ^~~
