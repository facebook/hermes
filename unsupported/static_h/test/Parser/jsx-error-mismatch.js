/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-jsx -dump-ast -pretty-json %s 2>&1 ) | %FileCheck %s --match-full-lines

<a></b>;
// CHECK: {{.*}}:10:4: error: Closing tag must match opening
// CHECK: <a></b>;
// CHECK:    ^~~~
// CHECK: {{.*}}:10:1: note: location of opening
// CHECK: <a></b>;
// CHECK: ^

<x:a></y:a>;
// CHECK: {{.*}}:18:6: error: Closing tag must match opening
// CHECK: <x:a></y:a>;
// CHECK:      ^~~~~~
// CHECK: {{.*}}:18:1: note: location of opening
// CHECK: <x:a></y:a>;
// CHECK: ^

<x.a></y>;
// CHECK: {{.*}}:26:6: error: Closing tag must match opening
// CHECK: <x.a></y>;
// CHECK:      ^~~~
// CHECK: {{.*}}:26:1: note: location of opening
// CHECK: <x.a></y>;
// CHECK: ^
