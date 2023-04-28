/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -Xparse-component-syntax -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

declare component Foo1(foo as bar: Foo);

// CHECK: {{.*}}:10:28: error: 'as' not allowed in component type parameter
// CHECK-NEXT: declare component Foo1(foo as bar: Foo);
// CHECK-NEXT:                            ^
